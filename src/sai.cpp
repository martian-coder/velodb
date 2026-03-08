/*
 * VeloDB: High-Performance Asynchronous Storage Engine
 * Copyright (c) 2026 Amit Nilajkar <amit.nilajkar@gmail.com>
 * 
 * This software and the associated Nexus Protocol (NXP) are the 
 * intellectual property of Amit Nilajkar.
 * Licensed under the MIT License. See LICENSE file for details.
 */

#include "velodb/sai.h"
#include <cassert>
#include <cmath>

#if defined(_MSC_VER)
#include <intrin.h>
static inline uint64_t popcnt(uint64_t x) { return __popcnt64(x); }
#else
static inline uint64_t popcnt(uint64_t x) { return __builtin_popcountll(x); }
#endif

namespace velodb {

// ---------------------------------------------------------------------
// Constructor – builds Elias‑Fano + bitmap from a sorted vector
// ---------------------------------------------------------------------
SAI::SAI(const std::vector<uint64_t>& keys)
    : universe_(keys.empty() ? 0 : keys.back() + 1), n_(keys.size()), keys_(keys)
{
    if (n_ == 0) {
        low_width_ = 0;
        low_mask_ = 0;
        return;
    }
    
    // low_width = floor(log2(universe/n))
    if (universe_ > n_) {
        low_width_ = static_cast<uint64_t>(std::log2(double(universe_) / n_));
    } else {
        low_width_ = 0;
    }
    if (low_width_ > 60) low_width_ = 60; // Safety cap
    low_mask_  = (1ULL << low_width_) - 1;

    // Allocate bit‑vectors
    size_t high_bits_len = (n_ + (1ULL << low_width_) - 1) >> low_width_;
    high_bits_.resize(high_bits_len, 0);
    low_bits_.resize((n_ * low_width_ + 63) / 64, 0);

    // Fill structures
    uint64_t prev = 0;
    for (size_t i = 0; i < n_; ++i) {
        uint64_t key = keys[i];
        uint64_t high = key >> low_width_;
        uint64_t low  = key & low_mask_;

        // set high‑bit at position (high + i)
        size_t pos = high + i;
        if ((pos >> 6) >= high_bits_.size()) {
            high_bits_.resize((pos >> 6) + 1, 0);
        }
        high_bits_[pos >> 6] |= 1ULL << (pos & 63);

        // pack low bits
        size_t low_pos = i * low_width_;
        size_t word = low_pos >> 6;
        size_t off  = low_pos & 63;
        if (word >= low_bits_.size()) {
            low_bits_.resize(word + 2, 0);
        }
        low_bits_[word] |= low << off;
        if (off + low_width_ > 64) {
            low_bits_[word+1] |= low >> (64 - off);
        }
    }
}

// ---------------------------------------------------------------------
// Rank – number of keys ≤ `key`
// ---------------------------------------------------------------------
size_t SAI::rank(uint64_t key) const noexcept {
    if (n_ == 0) return 0;
    
    uint64_t high = key >> low_width_;
    uint64_t low  = key & low_mask_;

    // count 1‑bits up to (high + ?)
    size_t pos = high + n_; // upper bound
    size_t word = pos >> 6;
    size_t off  = pos & 63;

    uint64_t cnt = 0;
    for (size_t i = 0; i < word && i < high_bits_.size(); ++i) cnt += popcnt(high_bits_[i]);
    if (word < high_bits_.size()) {
        cnt += popcnt(high_bits_[word] & ((1ULL << off) - 1));
    }

    // adjust for low part (binary search inside the block)
    // For simplicity we just linear‑scan the last block (tiny in practice)
    size_t start = cnt > 0 ? cnt-1 : 0;
    while (start < n_ && keys_[start] <= key) ++start;
    return start;
}

// ---------------------------------------------------------------------
// Select – key at index `idx` (0‑based)
// ---------------------------------------------------------------------
uint64_t SAI::select(size_t idx) const noexcept {
    assert(idx < n_);
    // Find the position of the idx‑th 1‑bit in high_bits_
    size_t cnt = 0;
    size_t word = 0;
    while (word < high_bits_.size() && cnt + popcnt(high_bits_[word]) <= idx) {
        cnt += popcnt(high_bits_[word]);
        ++word;
    }
    
    if (word >= high_bits_.size()) return 0; // fallback just in case
    
    // locate the exact bit inside the word
    uint64_t w = high_bits_[word];
    size_t bit = 0;
    while (cnt < idx) {
        w &= w - 1; // clear lowest set bit
        ++cnt;
        ++bit;
    }
    
    // Find the next set bit quickly since we cleared bits up to idx-1
#if defined(_MSC_VER)
    unsigned long trailing_zeros;
    if (_BitScanForward64(&trailing_zeros, w)) {
        bit = trailing_zeros;
    }
#else
    bit = __builtin_ctzll(w);
#endif

    size_t high = (word << 6) + bit - idx;
    // retrieve low part
    size_t low_pos = idx * low_width_;
    size_t widx = low_pos >> 6;
    size_t off  = low_pos & 63;
    uint64_t low = 0;
    if (widx < low_bits_.size()) {
        low = (low_bits_[widx] >> off) & low_mask_;
        if (off + low_width_ > 64 && (widx+1) < low_bits_.size()) {
            low |= (low_bits_[widx+1] << (64 - off)) & low_mask_;
        }
    }
    return (high << low_width_) | low;
}

// ---------------------------------------------------------------------
// Range – returns all keys in [lo,hi]
// ---------------------------------------------------------------------
std::vector<uint64_t> SAI::range(uint64_t lo, uint64_t hi) const {
    size_t start = (lo == 0) ? 0 : rank(lo - 1);
    size_t end   = rank(hi);
    std::vector<uint64_t> out;
    if (end > start) {
        out.reserve(end - start);
        for (size_t i = start; i < end; ++i) {
            out.push_back(select(i));
        }
    }
    return out;
}

} // namespace velodb

