/*
 * VeloDB: High-Performance Asynchronous Storage Engine
 * Copyright (c) 2026 Amit Nilajkar <amit.nilajkar@gmail.com>
 * 
 * This software and the associated Nexus Protocol (NXP) are the 
 * intellectual property of Amit Nilajkar.
 * Licensed under the MIT License. See LICENSE file for details.
 */

#include "velodb/snapshot.h"

namespace velodb {

bool Snapshot::get(uint64_t key, uint64_t& out) const noexcept {
    const LFNode* node = root_;
    while (node) {
        size_t idx = node->find(key);
        if (idx < node->keyCount && node->keys[idx] == key) {
            out = node->values[idx];
            return true;
        }
        if (node->children) node = node->children[idx];
        else break;
    }
    return false;
}

std::vector<std::pair<uint64_t, uint64_t>>
Snapshot::range(uint64_t lo, uint64_t hi) const noexcept {
    // Very naive implementation – walk the whole tree.
    // Production code would reuse the SAI; here we just demo the API.
    std::vector<std::pair<uint64_t, uint64_t>> out;
    // Recursive helper (lambda)
    std::function<void(const LFNode*)> dfs = [&](const LFNode* n) {
        if (!n) return;
        for (size_t i = 0; i < n->keyCount; ++i) {
            uint64_t k = n->keys[i];
            if (k >= lo && k <= hi) out.emplace_back(k, n->values[i]);
        }
        if (n->children) {
            for (size_t i = 0; i <= n->keyCount; ++i) dfs(n->children[i]);
        }
    };
    dfs(root_);
    return out;
}

} // namespace velodb

