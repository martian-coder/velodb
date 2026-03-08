/*
 * VeloDB: High-Performance Asynchronous Storage Engine
 * Copyright (c) 2026 Amit Nilajkar <amit.nilajkar@gmail.com>
 * 
 * This software and the associated Nexus Protocol (NXP) are the 
 * intellectual property of Amit Nilajkar.
 * Licensed under the MIT License. See LICENSE file for details.
 */

#pragma once
#include <cstdint>
#include <fstream>
#include <vector>
#include <shared_mutex>
#include <filesystem>

namespace velodb {

class HLSM {
public:
    explicit HLSM(const std::string& data_dir,
                  size_t hot_buffer_bytes = 64 << 20);
    ~HLSM();

    // Append a key/value pair to the hot buffer; flush when full.
    void append(uint64_t key, uint64_t value);

    // Iterate over the immutable log (used for recovery / snapshot)
    using LogEntry = std::pair<uint64_t, uint64_t>;
    std::vector<LogEntry> scan_all() const;
    std::vector<LogEntry> scan_hot() const;
    
    // Force a flush (used by snapshot)
    void flush();

    bool is_healthy() const { return healthy_; }
    size_t get_log_size() const;

private:
    std::string dir_;
    size_t      hot_capacity_;
    std::vector<uint8_t> hot_buffer_;   // simple binary buffer
    size_t               hot_used_;

    std::ofstream log_file_;
    mutable std::shared_mutex mu_; // protects buffer & file
    mutable bool healthy_ = true;

    void write_to_log(const uint8_t* data, size_t len);
};

} // namespace velodb

