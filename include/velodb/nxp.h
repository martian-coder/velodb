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

namespace velodb {

// Nexus Protocol (NXP) Header
#pragma pack(push, 1)
struct NXPHeader {
    uint32_t magic;         // 0x4E585021 ('NXP!')
    uint16_t version;       // 1
    uint8_t  command;       // Command ID
    uint32_t payload_size;  // Size of the following data
};
#pragma pack(pop)

enum class NXPCommand : uint8_t {
    PUT = 0x01,
    GET = 0x02,
    RANGE = 0x03,
    SNAPSHOT = 0x04,
    BACKUP = 0x05,
    STATS = 0x06,
    NXP_ERROR = 0xFF
};

static constexpr uint32_t NXP_MAGIC = 0x4E585021;
static constexpr uint16_t NXP_VERSION = 1;

} // namespace velodb

