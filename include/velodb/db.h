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
#include <memory>
#include <vector>
#include <shared_mutex>
#include <chrono>
#include <string>
#include "lf_btree.h"
#include "hlsm.h"
#include "sai.h"
#include "pwac.h"
#include "snapshot.h"

namespace velodb {

class DB {
public:
    // --------------------------------------------------------------------
    // Construction / destruction
    // --------------------------------------------------------------------
    explicit DB(const std::string& data_dir,
                size_t hot_buffer_bytes = 64 << 20,   // 64 MiB
                size_t pwac_width = 1 << 20,          // 1 M counters
                size_t pwac_depth = 5);
    ~DB();

    // --------------------------------------------------------------------
    // Core operations (all thread‑safe)
    // --------------------------------------------------------------------
    bool get(uint64_t key, uint64_t& out) const;
    void put(uint64_t key, uint64_t value);
    std::vector<std::pair<uint64_t, uint64_t>> range(uint64_t lo,
                                                     uint64_t hi) const;

    // --------------------------------------------------------------------
    // Snapshot & backup
    // --------------------------------------------------------------------
    std::shared_ptr<Snapshot> snapshot() const;
    void backup(const std::string& dst_path) const;

    // --------------------------------------------------------------------
    // Statistics (for monitoring)
    // --------------------------------------------------------------------
    struct Stats {
        uint64_t reads  = 0;
        uint64_t writes = 0;
        uint64_t bytes_written = 0;
        uint64_t bytes_read    = 0;
    };
    Stats stats() const { return stats_; }
    std::string get_metrics_json() const;

private:
    struct SplitResult {
        LFNode* left = nullptr;
        LFNode* right = nullptr;
        uint64_t split_key = 0;
    };

    SplitResult cow_insert(const LFNode* node, uint64_t key, uint64_t value);
    void collect_all(const LFNode* node, std::vector<uint64_t>& keys, std::vector<uint64_t>& values) const;
    void collect_range(const LFNode* node, uint64_t lo, uint64_t hi, std::vector<std::pair<uint64_t, uint64_t>>& results) const;
    LFNode* build_tree(const std::vector<uint64_t>& keys, const std::vector<uint64_t>& values);

    // Core data structures
    mutable std::atomic<LFNode*> root_;      // lock‑free B‑tree root
    mutable std::shared_mutex   root_mu_;   // for safe snapshot creation

    SlabArena   arena_;       // memory allocator for B-tree nodes
    HLSM        hlsm_;        // hot buffer + append‑only log
    SAI         sai_;         // succinct adaptive index (range scans)
    PWAC        pwac_;        // write‑amplification controller

    // Statistics (updated atomically)
    mutable std::atomic<Stats> stats_;
    std::chrono::steady_clock::time_point start_time_;

    // Configuration / Helpers
    static constexpr size_t MAX_LEAF_SIZE = 256;
    static constexpr size_t MAX_INTERNAL_SIZE = 256;
};

} // namespace velodb

