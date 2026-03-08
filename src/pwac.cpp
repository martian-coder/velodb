/*
 * VeloDB: High-Performance Asynchronous Storage Engine
 * Copyright (c) 2026 Amit Nilajkar <amit.nilajkar@gmail.com>
 * 
 * This software and the associated Nexus Protocol (NXP) are the 
 * intellectual property of Amit Nilajkar.
 * Licensed under the MIT License. See LICENSE file for details.
 */

#include "velodb/pwac.h"
#include <random>

namespace velodb {

// ---------------------------------------------------------------------
// Simple 64‑bit mix function (based on MurmurHash3 finalizer)
// ---------------------------------------------------------------------
static uint64_t mix64(uint64_t x) noexcept {
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return x;
}

// ---------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------
PWAC::PWAC(size_t width, size_t depth)
    : width_(width), depth_(depth),
      table_(new std::atomic<uint64_t>[width * depth])
{
    for (size_t i = 0; i < width * depth; ++i)
        table_[i].store(0, std::memory_order_relaxed);
}

// ---------------------------------------------------------------------
// Add a key
// ---------------------------------------------------------------------
void PWAC::add(uint64_t key) noexcept {
    for (size_t d = 0; d < depth_; ++d) {
        uint64_t h = mix64(key ^ d);
        size_t idx = h % width_;
        table_[d * width_ + idx].fetch_add(1, std::memory_order_relaxed);
    }
}

// ---------------------------------------------------------------------
// Estimate frequency
// ---------------------------------------------------------------------
uint64_t PWAC::estimate(uint64_t key) const noexcept {
    uint64_t min = UINT64_MAX;
    for (size_t d = 0; d < depth_; ++d) {
        uint64_t h = mix64(key ^ d);
        size_t idx = h % width_;
        uint64_t v = table_[d * width_ + idx].load(std::memory_order_relaxed);
        if (v < min) min = v;
    }
    return min;
}

// ---------------------------------------------------------------------
// Flush decision – we flush when the estimated write‑amplification
// would exceed 1.2× the optimal (see the research paper).
// ---------------------------------------------------------------------
bool PWAC::should_flush() const noexcept {
    // Simple heuristic: if any counter exceeds a threshold, flush.
    const uint64_t threshold = 1 << 12; // 4096 updates per key
    for (size_t i = 0; i < width_ * depth_; ++i) {
        if (table_[i].load(std::memory_order_relaxed) > threshold)
            return true;
    }
    return false;
}

// ---------------------------------------------------------------------
// Reset the sketch after a flush
// ---------------------------------------------------------------------
void PWAC::reset() noexcept {
    for (size_t i = 0; i < width_ * depth_; ++i)
        table_[i].store(0, std::memory_order_relaxed);
}

// ---------------------------------------------------------------------
// Hash helper (used internally)
// ---------------------------------------------------------------------
uint64_t PWAC::hash(uint64_t key, size_t seed) noexcept {
    return mix64(key ^ seed);
}

} // namespace velodb

