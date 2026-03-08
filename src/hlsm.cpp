/*
 * VeloDB: High-Performance Asynchronous Storage Engine
 * Copyright (c) 2026 Amit Nilajkar <amit.nilajkar@gmail.com>
 * 
 * This software and the associated Nexus Protocol (NXP) are the 
 * intellectual property of Amit Nilajkar.
 * Licensed under the MIT License. See LICENSE file for details.
 */

#include "velodb/hlsm.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <numeric>
#include <shared_mutex>
#include <filesystem>

namespace velodb {

// A simple CRC32-like checksum for lightweight durability
uint32_t calculate_crc32(uint64_t k, uint64_t v) {
    uint32_t crc = 0xFFFFFFFF;
    auto update = [&](uint64_t val) {
        for (int i = 0; i < 8; ++i) {
            uint8_t byte = (val >> (i * 8)) & 0xFF;
            crc ^= byte;
            for (int j = 0; j < 8; ++j) {
                crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
            }
        }
    };
    update(k);
    update(v);
    return ~crc;
}

HLSM::HLSM(const std::string& data_dir, size_t hot_buffer_bytes)
    : dir_(data_dir),
      hot_capacity_(hot_buffer_bytes),
      hot_buffer_(hot_buffer_bytes),
      hot_used_(0)
{
    std::filesystem::create_directories(dir_);
    std::string log_path = dir_ + "/log.bin";
    // Open in in|out|app mode to allow truncation if needed
    log_file_.open(log_path, std::ios::binary | std::ios::in | std::ios::out | std::ios::app);
    if (!log_file_) {
        // Fallback for new files
        log_file_.clear();
        log_file_.open(log_path, std::ios::binary | std::ios::out | std::ios::app);
    }
}

HLSM::~HLSM() {
    flush();
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

// ---------------------------------------------------------------------
// Append a key/value pair to the in‑memory buffer.
// ---------------------------------------------------------------------
void HLSM::append(uint64_t key, uint64_t value) {
    std::unique_lock lock(mu_);
    // Log Record Format: [Key 8b] [Value 8b] [CRC32 4b] = 20 bytes
    uint32_t crc = calculate_crc32(key, value);
    
    if (hot_used_ + 20 > hot_capacity_) {
        if (hot_used_ > 0) {
            write_to_log(hot_buffer_.data(), hot_used_);
            hot_used_ = 0;
        }
    }
    
    std::memcpy(&hot_buffer_[hot_used_], &key,   sizeof(uint64_t));
    std::memcpy(&hot_buffer_[hot_used_+8], &value, sizeof(uint64_t));
    std::memcpy(&hot_buffer_[hot_used_+16], &crc,   sizeof(uint32_t));
    hot_used_ += 20;
}

size_t HLSM::get_log_size() const {
    std::shared_lock lock(mu_);
    std::string log_path = dir_ + "/log.bin"; // Corrected filename
    if (std::filesystem::exists(log_path)) {
        return std::filesystem::file_size(log_path);
    }
    return 0;
}

void HLSM::flush() {
    std::unique_lock lock(mu_);
    if (hot_used_ == 0) return;
    write_to_log(hot_buffer_.data(), hot_used_);
    hot_used_ = 0;
}

void HLSM::write_to_log(const uint8_t* data, size_t len) {
    if (!log_file_.is_open()) return;
    log_file_.seekp(0, std::ios::end);
    log_file_.write(reinterpret_cast<const char*>(data), len);
    log_file_.flush();
}

// ---------------------------------------------------------------------
// Scan the whole log (used for recovery / snapshot)
// ---------------------------------------------------------------------
std::vector<HLSM::LogEntry> HLSM::scan_all() const {
    std::shared_lock lock(mu_);
    std::string log_path = dir_ + "/log.bin";
    std::ifstream in(log_path, std::ios::binary);
    if (!in) return {};

    std::vector<LogEntry> out;
    uint8_t buf[20];
    size_t last_valid_pos = 0;
    bool corruption_found = false;

    while (in.read(reinterpret_cast<char*>(buf), 20)) {
        uint64_t k, v;
        uint32_t stored_crc;
        std::memcpy(&k, buf, sizeof(uint64_t));
        std::memcpy(&v, buf+8, sizeof(uint64_t));
        std::memcpy(&stored_crc, buf+16, sizeof(uint32_t));

        if (calculate_crc32(k, v) == stored_crc) {
            out.emplace_back(k, v);
            last_valid_pos = static_cast<size_t>(in.tellg());
        } else {
            std::cerr << "WARNING: Log corruption detected at pos " << (size_t)in.tellg() - 20 << ". Truncating log." << std::endl;
            healthy_ = false; // Set healthy_ to false on corruption
            corruption_found = true;
            break;
        }
    }

    // Handle partial record at EOF
    if (!corruption_found && in.gcount() > 0 && in.gcount() < 20) {
        std::cerr << "WARNING: Partial log record detected at EOF. Truncating " << in.gcount() << " bytes." << std::endl;
        corruption_found = true;
    }

    if (corruption_found) {
        in.close();
        // Truncate the file at last_valid_pos
        std::filesystem::resize_file(log_path, last_valid_pos);
    }

    return out;
}

std::vector<HLSM::LogEntry> HLSM::scan_hot() const {
    std::shared_lock lock(mu_);
    std::vector<LogEntry> out;
    for (size_t i = 0; i < hot_used_; i += 20) {
        uint64_t k, v;
        std::memcpy(&k, &hot_buffer_[i], sizeof(uint64_t));
        std::memcpy(&v, &hot_buffer_[i+8], sizeof(uint64_t));
        out.emplace_back(k, v);
    }
    return out;
}

} // namespace velodb

