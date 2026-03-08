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
#include <vector>
#include <atomic>
#include <cstring>
#include <memory>
#include <mutex>

namespace velodb {

// ------------------------------------------------------------------------
// Immutable slab – a node of the B‑tree that never mutates after creation.
// ------------------------------------------------------------------------
struct LFNode {
    const uint64_t* keys;        // sorted, read‑only
    const uint64_t* values;      // aligned with keys
    const LFNode**   children;    // nullptr for leaf
    const size_t    keyCount;
    const uint64_t  version;      // monotonically increasing

    // Helper: binary search inside this slab (returns index or keyCount)
    size_t find(uint64_t key) const noexcept {
        size_t lo = 0, hi = keyCount;
        while (lo < hi) {
            size_t mid = (lo + hi) >> 1;
            if (keys[mid] < key) lo = mid + 1;
            else hi = mid;
        }
        return lo;
    }

    LFNode(const uint64_t* k, const uint64_t* v, const LFNode** c, size_t count, uint64_t ver)
        : keys(k), values(v), children(c), keyCount(count), version(ver) {}
};

// ------------------------------------------------------------------------
// Allocation helpers – all slabs are allocated from a custom arena to avoid
// fragmentation and to keep them cache‑line aligned.
// ------------------------------------------------------------------------
class SlabArena {
public:
    struct Block {
        uint8_t* mem;
        size_t   used;
        size_t   capacity;
    };

    SlabArena(size_t block_bytes = 64 << 20); // 64 MiB by default
    ~SlabArena();

    // Allocate a slab for `n` keys (leaf) or `n` children (internal)
    LFNode* allocate_leaf(const std::vector<uint64_t>& keys,
                          const std::vector<uint64_t>& values);
    LFNode* allocate_internal(const std::vector<uint64_t>& keys,
                              const std::vector<LFNode*>& children);

    size_t used_bytes() const;

public:
    std::vector<Block> blocks_;
    mutable std::mutex mu_; // protects block allocation (rare)
};

} // namespace velodb

