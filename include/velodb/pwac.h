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
#include <cmath>
#include <memory>

namespace velodb {

// ------------------------------------------------------------------------
// Count‑Min Sketch for hot‑key prediction (write‑amplification control)
// ------------------------------------------------------------------------
class PWAC {
public:
    PWAC(size_t width = 1 << 20, size_t depth = 5);

    // Increment the counter for a key
    void add(uint64_t key) noexcept;

    // Estimate frequency of a key
    uint64_t estimate(uint64_t key) const noexcept;

    // Decide whether the hot buffer should be flushed (returns true if
    // estimated write‑amplification would exceed the target)
    bool should_flush() const noexcept;

    // Reset the sketch (called after a flush)
    void reset() noexcept;

private:
    const size_t width_;
    const size_t depth_;
    std::unique_ptr<std::atomic<uint64_t>[]> table_;

    // Simple 64‑bit hash (MurmurHash3‑x64‑128 style)
    static uint64_t hash(uint64_t key, size_t seed) noexcept;
};

} // namespace velodb

