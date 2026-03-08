/*
 * VeloDB: High-Performance Asynchronous Storage Engine
 * Copyright (c) 2026 Amit Nilajkar <amit.nilajkar@gmail.com>
 * 
 * This software and the associated Nexus Protocol (NXP) are the 
 * intellectual property of Amit Nilajkar.
 * Licensed under the MIT License. See LICENSE file for details.
 */

#include "velodb/lf_btree.h"
#include <cstdlib>
#include <cstring>
#include <new>
#include <mutex>

namespace velodb {

// ---------------------------------------------------------------------
// SlabArena implementation
// ---------------------------------------------------------------------
SlabArena::SlabArena(size_t block_bytes) {
    Block blk{static_cast<uint8_t*>(std::malloc(block_bytes)), 0, block_bytes};
    blocks_.push_back(blk);
}
SlabArena::~SlabArena() {
    for (auto& b : blocks_) std::free(b.mem);
}

size_t SlabArena::used_bytes() const {
    std::lock_guard<std::mutex> lock(mu_);
    size_t total = 0;
    for (auto& b : blocks_) total += b.used;
    return total;
}

// Allocate a new block when the current one is full
static SlabArena::Block* allocate_block(std::vector<SlabArena::Block>& vec,
                                       size_t block_bytes) {
    SlabArena::Block blk{static_cast<uint8_t*>(std::malloc(block_bytes)), 0,
                         block_bytes};
    vec.push_back(blk);
    return &vec.back();
}

// Helper to get a pointer to a fresh region inside the arena
static uint8_t* arena_alloc(SlabArena& arena,
                            size_t bytes) {
    std::lock_guard<std::mutex> lock(arena.mu_);
    if (arena.blocks_.back().used + bytes > arena.blocks_.back().capacity) {
        allocate_block(arena.blocks_, arena.blocks_.back().capacity);
    }
    SlabArena::Block& cur = arena.blocks_.back();
    uint8_t* ptr = cur.mem + cur.used;
    cur.used += bytes;
    return ptr;
}

// ---------------------------------------------------------------------
// Leaf allocation
// ---------------------------------------------------------------------
LFNode* SlabArena::allocate_leaf(const std::vector<uint64_t>& keys,
                                 const std::vector<uint64_t>& values) {
    size_t n = keys.size();
    size_t key_bytes   = n * sizeof(uint64_t);
    size_t value_bytes = n * sizeof(uint64_t);
    size_t node_bytes  = sizeof(LFNode);

    uint8_t* mem = arena_alloc(*this, node_bytes + key_bytes + value_bytes);
    uint64_t* key_ptr   = reinterpret_cast<uint64_t*>(mem + node_bytes);
    uint64_t* value_ptr = reinterpret_cast<uint64_t*>(mem + node_bytes + key_bytes);

    std::memcpy(key_ptr,   keys.data(),   key_bytes);
    std::memcpy(value_ptr, values.data(), value_bytes);

    LFNode* node = new (mem) LFNode(key_ptr, value_ptr, nullptr, n, 0);
    return node;
}

// ---------------------------------------------------------------------
// Internal node allocation (children are already allocated)
// ---------------------------------------------------------------------
LFNode* SlabArena::allocate_internal(const std::vector<uint64_t>& keys,
                                    const std::vector<LFNode*>& children) {
    size_t n = keys.size();
    size_t m = children.size();
    size_t key_bytes   = n * sizeof(uint64_t);
    size_t child_bytes = m * sizeof(LFNode*);
    size_t node_bytes  = sizeof(LFNode);

    uint8_t* mem = arena_alloc(*this, node_bytes + key_bytes + child_bytes);
    uint64_t* key_ptr   = reinterpret_cast<uint64_t*>(mem + node_bytes);
    const LFNode** child_ptr = reinterpret_cast<const LFNode**>(mem + node_bytes + key_bytes);

    std::memcpy(key_ptr,   keys.data(),   key_bytes);
    std::memcpy(child_ptr, children.data(), child_bytes);

    LFNode* node = new (mem) LFNode(key_ptr, nullptr, child_ptr, n, 0);
    return node;
}

} // namespace velodb

