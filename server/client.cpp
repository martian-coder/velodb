/*
 * VeloDB: High-Performance Asynchronous Storage Engine
 * Copyright (c) 2026 Amit Nilajkar <amit.nilajkar@gmail.com>
 * 
 * This software and the associated Nexus Protocol (NXP) are the 
 * intellectual property of Amit Nilajkar.
 * Licensed under the MIT License. See LICENSE file for details.
 */

#include <asio.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include "velodb/nxp.h"

using asio::ip::tcp;
using namespace velodb;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: velodb-client [--port <port>] <cmd> <args...>" << std::endl;
        return 1;
    }

    std::string port = "9000";
    int cmd_idx = 1;
    if (std::string(argv[1]) == "--port" && argc >= 4) {
        port = argv[2];
        cmd_idx = 3;
    }

    std::string op = argv[cmd_idx];
    for (auto& c : op) c = std::toupper(c);

    try {
        asio::io_context io_context;
        asio::ip::tcp::socket conn(io_context);
        asio::ip::tcp::resolver resolver(io_context);
        
        auto endpoints = resolver.resolve("localhost", port);
        std::cout << "Connecting to localhost:" << port << "..." << std::endl;
        asio::connect(conn, endpoints);
        std::cout << "Connected!" << std::endl;

        NXPHeader header;
        header.magic = NXP_MAGIC;
        header.version = NXP_VERSION;
        std::vector<char> payload;

        if (op == "PUT" && argc >= cmd_idx + 3) {
            header.command = static_cast<uint8_t>(NXPCommand::PUT);
            uint64_t k = std::stoull(argv[cmd_idx + 1]);
            uint64_t v = std::stoull(argv[cmd_idx + 2]);
            payload.resize(16);
            std::memcpy(payload.data(), &k, 8);
            std::memcpy(payload.data() + 8, &v, 8);
        } else if (op == "GET" && argc >= cmd_idx + 2) {
            header.command = static_cast<uint8_t>(NXPCommand::GET);
            uint64_t k = std::stoull(argv[cmd_idx + 1]);
            payload.resize(8);
            std::memcpy(payload.data(), &k, 8);
        } else if (op == "RANGE" && argc >= cmd_idx + 3) {
            header.command = static_cast<uint8_t>(NXPCommand::RANGE);
            uint64_t lo = std::stoull(argv[cmd_idx + 1]);
            uint64_t hi = std::stoull(argv[cmd_idx + 2]);
            payload.resize(16);
            std::memcpy(payload.data(), &lo, 8);
            std::memcpy(payload.data() + 8, &hi, 8);
        } else if (op == "SNAPSHOT") {
            header.command = static_cast<uint8_t>(NXPCommand::SNAPSHOT);
        } else if (op == "STATS") {
            header.command = static_cast<uint8_t>(NXPCommand::STATS);
        } else {
            std::cerr << "Invalid command or arguments." << std::endl;
            return 1;
        }

        header.payload_size = static_cast<uint32_t>(payload.size());
        
        // Write header and payload
        asio::write(conn, asio::buffer(&header, sizeof(header)));
        if (!payload.empty()) {
            asio::write(conn, asio::buffer(payload));
        }

        // Read response header
        NXPHeader resp_header;
        asio::read(conn, asio::buffer(&resp_header, sizeof(resp_header)));

        if (resp_header.payload_size > 0) {
            std::vector<char> resp_payload(resp_header.payload_size);
            asio::read(conn, asio::buffer(resp_payload));

            if (static_cast<NXPCommand>(resp_header.command) == NXPCommand::NXP_ERROR) {
                std::cout << "ERROR: " << std::string(resp_payload.begin(), resp_payload.end()) << std::endl;
            } else if (op == "GET") {
                uint64_t v;
                std::memcpy(&v, resp_payload.data(), 8);
                std::cout << "VALUE " << v << std::endl;
            } else if (op == "RANGE") {
                uint32_t count;
                std::memcpy(&count, resp_payload.data(), 4);
                std::cout << "RANGE " << count << std::endl;
                for (uint32_t i = 0; i < count; ++i) {
                    uint64_t k, v;
                    std::memcpy(&k, resp_payload.data() + 4 + i * 16, 8);
                    std::memcpy(&v, resp_payload.data() + 4 + i * 16 + 8, 8);
                    std::cout << k << " " << v << std::endl;
                }
            } else if (static_cast<NXPCommand>(resp_header.command) == NXPCommand::STATS) {
                std::string stats_json(resp_payload.begin(), resp_payload.end());
                std::cout << stats_json << std::endl;
            } else {
                std::cout << std::string(resp_payload.begin(), resp_payload.end()) << std::endl;
            }
        } else {
            std::cout << "OK" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Client Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

