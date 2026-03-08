/*
 * VeloDB: High-Performance Asynchronous Storage Engine
 * Copyright (c) 2026 Amit Nilajkar <amit.nilajkar@gmail.com>
 * 
 * This software and the associated Nexus Protocol (NXP) are the 
 * intellectual property of Amit Nilajkar.
 * Licensed under the MIT License. See LICENSE file for details.
 */

#include "velodb/db.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>

namespace velodb {

DB::DB(const std::string& data_dir,
         size_t hot_buffer_bytes,
         size_t pwac_width,
         size_t pwac_depth)
    : root_(nullptr),
      hlsm_(data_dir, hot_buffer_bytes),
      sai_(),
      pwac_(pwac_width, pwac_depth),
      stats_({0,0,0,0}),
      start_time_(std::chrono::steady_clock::now())
{
    // Recover from existing log (if any)
    auto entries = hlsm_.scan_all();
    for (auto& e : entries) {
        // Re‑play each entry – this also builds the SAI
        put(e.first, e.second);
    }
}

DB::~DB() {
    // Ensure a final flush
    hlsm_.flush();
}

// ---------------------------------------------------------------------
// GET – lock‑free read
// ---------------------------------------------------------------------
bool DB::get(uint64_t key, uint64_t& out) const {
    const LFNode* root = root_.load(std::memory_order_acquire);
    while (root) {
        size_t idx = root->find(key);
        if (idx < root->keyCount && root->keys[idx] == key) {
            out = root->values[idx];
            Stats curr = stats_.load(std::memory_order_relaxed);
            do {
                Stats next = curr;
                next.reads += 1;
                if (stats_.compare_exchange_weak(curr, next, std::memory_order_relaxed)) break;
            } while (true);
            return true;
        }
        if (root->children) {
            root = root->children[idx];
        } else {
            break;
        }
    }
    return false;
}

void DB::collect_all(const LFNode* node, std::vector<uint64_t>& keys, std::vector<uint64_t>& values) const {
    if (!node) return;
    if (node->children == nullptr) {
        for (size_t i = 0; i < node->keyCount; ++i) {
            keys.push_back(node->keys[i]);
            values.push_back(node->values[i]);
        }
    } else {
        for (size_t i = 0; i <= node->keyCount; ++i) {
            collect_all(node->children[i], keys, values);
        }
    }
}

LFNode* DB::build_tree(const std::vector<uint64_t>& keys, const std::vector<uint64_t>& values) {
    if (keys.empty()) return nullptr;
    if (keys.size() <= MAX_LEAF_SIZE) {
        return arena_.allocate_leaf(keys, values);
    }

    // Build leaves
    std::vector<LFNode*> nodes;
    for (size_t i = 0; i < keys.size(); i += MAX_LEAF_SIZE) {
        size_t end = std::min(i + MAX_LEAF_SIZE, keys.size());
        std::vector<uint64_t> k(keys.begin() + i, keys.begin() + end);
        std::vector<uint64_t> v(values.begin() + i, values.begin() + end);
        nodes.push_back(arena_.allocate_leaf(k, v));
    }

    // Build internal levels
    while (nodes.size() > 1) {
        std::vector<LFNode*> next_level;
        for (size_t i = 0; i < nodes.size(); i += MAX_INTERNAL_SIZE) {
            size_t end = std::min(i + MAX_INTERNAL_SIZE, nodes.size());
            std::vector<LFNode*> children(nodes.begin() + i, nodes.begin() + end);
            
            std::vector<uint64_t> sep_keys;
            for (size_t j = 1; j < children.size(); ++j) {
                // Separator is the first key of the right sibling
                const LFNode* child = children[j];
                while (child->children) child = child->children[0];
                sep_keys.push_back(child->keys[0]);
            }
            next_level.push_back(arena_.allocate_internal(sep_keys, children));
        }
        nodes = std::move(next_level);
    }
    return nodes[0];
}

DB::SplitResult DB::cow_insert(const LFNode* node, uint64_t key, uint64_t value) {
    if (!node) {
        return { arena_.allocate_leaf({key}, {value}), nullptr, 0 };
    }

    if (node->children == nullptr) { // Leaf
        std::vector<uint64_t> keys, vals;
        bool found = false;
        for (size_t i = 0; i < node->keyCount; ++i) {
            if (node->keys[i] == key) {
                keys.push_back(key);
                vals.push_back(value);
                found = true;
            } else {
                keys.push_back(node->keys[i]);
                vals.push_back(node->values[i]);
            }
        }
        if (!found) {
            auto it = std::lower_bound(keys.begin(), keys.end(), key);
            size_t d = std::distance(keys.begin(), it);
            keys.insert(it, key);
            vals.insert(vals.begin() + d, value);
        }

        if (keys.size() <= MAX_LEAF_SIZE) {
            return { arena_.allocate_leaf(keys, vals), nullptr, 0 };
        } else {
            size_t mid = keys.size() / 2;
            std::vector<uint64_t> l_k(keys.begin(), keys.begin() + mid);
            std::vector<uint64_t> l_v(vals.begin(), vals.begin() + mid);
            std::vector<uint64_t> r_k(keys.begin() + mid, keys.end());
            std::vector<uint64_t> r_v(vals.begin() + mid, vals.end());
            return { arena_.allocate_leaf(l_k, l_v), arena_.allocate_leaf(r_k, r_v), r_k[0] };
        }
    } else { // Internal
        size_t idx = node->find(key);
        SplitResult res = cow_insert(node->children[idx], key, value);
        
        std::vector<uint64_t> keys(node->keys, node->keys + node->keyCount);
        std::vector<LFNode*> children;
        for (size_t i = 0; i <= node->keyCount; ++i) {
            children.push_back(const_cast<LFNode*>(node->children[i]));
        }

        children[idx] = res.left;
        if (res.right) {
            keys.insert(keys.begin() + idx, res.split_key);
            children.insert(children.begin() + idx + 1, res.right);
        }

        if (keys.size() <= MAX_INTERNAL_SIZE) {
            return { arena_.allocate_internal(keys, children), nullptr, 0 };
        } else {
            size_t mid = keys.size() / 2;
            std::vector<uint64_t> l_k(keys.begin(), keys.begin() + mid);
            // mid keys -> mid+1 children [0...mid]
            std::vector<LFNode*> l_c(children.begin(), children.begin() + mid + 1);
            
            uint64_t s_k = keys[mid];
            
            std::vector<uint64_t> r_k(keys.begin() + mid + 1, keys.end());
            // remaining children [mid+1...end]
            std::vector<LFNode*> r_c(children.begin() + mid + 1, children.end());
            
            return { arena_.allocate_internal(l_k, l_c), arena_.allocate_internal(r_k, r_c), s_k };
        }
    }
}

// ---------------------------------------------------------------------
// PUT – copy‑on‑write path
// ---------------------------------------------------------------------
void DB::put(uint64_t key, uint64_t value) {
    hlsm_.append(key, value);
    pwac_.add(key);
    
    {
        std::unique_lock<std::shared_mutex> w_lock(root_mu_);
        if (pwac_.should_flush()) {
            hlsm_.flush();
            pwac_.reset();
        }
    }

    LFNode* old_root = root_.load(std::memory_order_acquire);
    while (true) {
        SplitResult res = cow_insert(old_root, key, value);
        LFNode* new_root = nullptr;
        if (res.right) {
            new_root = arena_.allocate_internal({res.split_key}, {res.left, res.right});
        } else {
            new_root = res.left;
        }

        if (root_.compare_exchange_weak(old_root, new_root,
                                            std::memory_order_release,
                                            std::memory_order_relaxed)) {
            break;
        }
        std::this_thread::yield();
    }

    // Periodic SAI and PWAC maintenance
    if (pwac_.should_flush()) {
        std::unique_lock<std::shared_mutex> w_lock(root_mu_);
        if (pwac_.should_flush()) { // double check
            hlsm_.flush();
            pwac_.reset();
            
            // Rebuild SAI after flush
            std::vector<uint64_t> all_keys, all_vals;
            collect_all(root_.load(std::memory_order_acquire), all_keys, all_vals);
            sai_ = SAI(all_keys);
        }
    }

    Stats curr = stats_.load(std::memory_order_relaxed);
    Stats next;
    do {
        next = curr;
        next.writes += 1;
        next.bytes_written += 8;
    } while (!stats_.compare_exchange_weak(curr, next, std::memory_order_relaxed));
}

void DB::collect_range(const LFNode* node, uint64_t lo, uint64_t hi, std::vector<std::pair<uint64_t, uint64_t>>& results) const {
    if (!node) return;
    if (node->children == nullptr) {
        for (size_t i = 0; i < node->keyCount; ++i) {
            if (node->keys[i] >= lo && node->keys[i] <= hi) {
                results.emplace_back(node->keys[i], node->values[i]);
            }
        }
    } else {
        for (size_t i = 0; i < node->keyCount; ++i) {
            if (lo <= node->keys[i]) {
                collect_range(node->children[i], lo, hi, results);
            }
            if (node->keys[i] >= lo && node->keys[i] <= hi) {
                // Internal nodes don't store values in this B-tree?
                // Wait, my build_tree puts all keys in leaves.
            }
            if (hi < node->keys[i]) break;
        }
        if (hi >= node->keys[node->keyCount - 1]) {
            collect_range(node->children[node->keyCount], lo, hi, results);
        }
    }
}

// ---------------------------------------------------------------------
// RANGE – uses the SAI for fast rank/select
// ---------------------------------------------------------------------
std::vector<std::pair<uint64_t, uint64_t>>
DB::range(uint64_t lo, uint64_t hi) const {
    std::vector<std::pair<uint64_t, uint64_t>> result;
    std::shared_lock<std::shared_mutex> r_lock(root_mu_);
    
    // Attempt SAI first
    auto k_indices = sai_.range(lo, hi);
    if (!k_indices.empty()) {
        for (uint64_t k : k_indices) {
            uint64_t v;
            if (get(k, v)) result.emplace_back(k, v);
        }
    } else {
        // Fallback to direct B-tree scan if SAI is empty
        collect_range(root_.load(std::memory_order_acquire), lo, hi, result);
        std::sort(result.begin(), result.end());
    }
    
    return result;
}

// ---------------------------------------------------------------------
// SNAPSHOT – copy‑on‑write of the root pointer
// ---------------------------------------------------------------------
std::shared_ptr<Snapshot> DB::snapshot() const {
    // Acquire a shared lock to guarantee the root won't be reclaimed while
    // we copy it (the root itself is immutable, so this is cheap).
    std::shared_lock lock(root_mu_);
    LFNode* root = root_.load(std::memory_order_acquire);
    return std::make_shared<Snapshot>(root);
}

// ---------------------------------------------------------------------
// BACKUP – write a consistent copy of the log to `dst_path`
// ---------------------------------------------------------------------
void DB::backup(const std::string& dst_path) const {
    std::ofstream out(dst_path, std::ios::binary);
    auto entries = hlsm_.scan_all();
    for (auto& e : entries) {
        out.write(reinterpret_cast<const char*>(&e.first), sizeof(uint64_t));
        out.write(reinterpret_cast<const char*>(&e.second), sizeof(uint64_t));
    }
    out.close();
}

std::string DB::get_metrics_json() const {
    auto now = std::chrono::steady_clock::now();
    uint64_t uptime_sec = std::chrono::duration_cast<std::chrono::seconds>(now - start_time_).count();
    
    Stats s = stats_.load(std::memory_order_relaxed);
    uint64_t total_ops = s.reads + s.writes;
    double throughput = uptime_sec > 0 ? (double)total_ops / uptime_sec : (double)total_ops;

    std::ostringstream oss;
    oss << "{"
        << "\"total_reads\":" << s.reads << ","
        << "\"total_writes\":" << s.writes << ","
        << "\"total_ops\":" << total_ops << ","
        << "\"throughput_ops_sec\":" << std::fixed << std::setprecision(2) << throughput << ","
        << "\"memory_used_bytes\":" << arena_.used_bytes() << ","
        << "\"wal_size_bytes\":" << hlsm_.get_log_size() << ","
        << "\"wal_healthy\":" << (hlsm_.is_healthy() ? "true" : "false") << ","
        << "\"uptime_seconds\":" << uptime_sec
        << "}";
    return oss.str();
}

} // namespace velodb

