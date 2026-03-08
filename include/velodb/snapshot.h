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
#include <functional>
#include "lf_btree.h"

namespace velodb {

class Snapshot {
public:
    explicit Snapshot(LFNode* root) : root_(root) {}

    // Read‑only view – same API as DB but works on a frozen root.
    bool get(uint64_t key, uint64_t& out) const noexcept;
    std::vector<std::pair<uint64_t, uint64_t>> range(uint64_t lo,
                                                     uint64_t hi) const noexcept;

private:
    LFNode* const root_;   // immutable – safe to read without locks
};

} // namespace velodb

