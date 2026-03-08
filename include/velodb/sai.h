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
#include <bitset>
#include <cassert>

namespace velodb {

// ------------------------------------------------------------------------
// Simple Elias‑Fano + bitmap implementation for monotonic keys.
// ------------------------------------------------------------------------
class SAI {
public:
    // Build from a sorted vector of keys (must be monotonic)
    explicit SAI(const std::vector<uint64_t>& keys);
    SAI() : universe_(0), n_(0), low_mask_(0), low_width_(0) {} // default ctor logic? Wait, the cpp doesn't have it but default is used in db

    // O(1) rank: number of keys ≤ `key`
    size_t rank(uint64_t key) const noexcept;

    // O(1) select: key at position `idx` (0‑based)
    uint64_t select(size_t idx) const noexcept;

    // Range scan – returns all keys in [lo,hi] (inclusive)
    std::vector<uint64_t> range(uint64_t lo, uint64_t hi) const;

private:
    // Parameters
    uint64_t  universe_;   // max key value + 1 (2⁶⁴)
    size_t    n_;           // number of keys

    // Elias‑Fano representation
    std::vector<uint64_t> high_bits_; // bit‑vector for high part
    std::vector<uint64_t> low_bits_;  // packed low part
    uint64_t               low_mask_;  // (1 << low_width) - 1
    uint64_t               low_width_; // floor(log2(universe/n))
    std::vector<uint64_t> keys_; // Need to store keys for rank last block scan

    // Helper to extract a low part
    uint64_t low(uint64_t key) const noexcept {
        return key & low_mask_;
    }
};

} // namespace velodb

