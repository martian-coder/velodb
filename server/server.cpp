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
#include <memory>
#include <string>
#include <vector>
#include <cstring>
#include <thread>
#include "velodb/db.h"
#include "velodb/nxp.h"
#include "velodb/banner.h"

using asio::ip::tcp;
using namespace velodb;

class NXPSession : public std::enable_shared_from_this<NXPSession> {
public:
    NXPSession(tcp::socket socket, DB& db)
        : socket_(std::move(socket)), db_(db) {}

    void start() {
        do_read_header();
    }

private:
    void do_read_header() {
        auto self(shared_from_this());
        asio::async_read(socket_, asio::buffer(&header_, sizeof(NXPHeader)),
            [this, self](asio::error_code ec, std::size_t /*length*/) {
                if (!ec) {
                    if (header_.magic != NXP_MAGIC || header_.version != NXP_VERSION) {
                        std::cerr << "Invalid NXP handshake" << std::endl;
                        return;
                    }
                    if (header_.payload_size > 0) {
                        payload_.resize(header_.payload_size);
                        do_read_payload();
                    } else {
                        handle_command();
                    }
                }
            });
    }

    void do_read_payload() {
        auto self(shared_from_this());
        asio::async_read(socket_, asio::buffer(payload_.data(), payload_.size()),
            [this, self](asio::error_code ec, std::size_t /*length*/) {
                if (!ec) {
                    handle_command();
                }
            });
    }

    void handle_command() {
        try {
            switch (static_cast<NXPCommand>(header_.command)) {
                case NXPCommand::PUT: {
                    if (payload_.size() < 16) break;
                    uint64_t k, v;
                    std::memcpy(&k, payload_.data(), 8);
                    std::memcpy(&v, payload_.data() + 8, 8);
                    db_.put(k, v);
                    send_response(NXPCommand::PUT, "OK");
                    break;
                }
                case NXPCommand::GET: {
                    if (payload_.size() < 8) break;
                    uint64_t k, v;
                    std::memcpy(&k, payload_.data(), 8);
                    if (db_.get(k, v)) {
                        char res[8];
                        std::memcpy(res, &v, 8);
                        send_response(NXPCommand::GET, std::string(res, 8));
                    } else {
                        send_response(NXPCommand::NXP_ERROR, "NOT_FOUND");
                    }
                    break;
                }
                case NXPCommand::RANGE: {
                    if (payload_.size() < 16) break;
                    uint64_t lo, hi;
                    std::memcpy(&lo, payload_.data(), 8);
                    std::memcpy(&hi, payload_.data() + 8, 8);
                    auto vec = db_.range(lo, hi);
                    
                    std::vector<char> res_buf;
                    uint32_t count = static_cast<uint32_t>(vec.size());
                    res_buf.resize(4 + count * 16);
                    std::memcpy(res_buf.data(), &count, 4);
                    for (size_t i = 0; i < count; ++i) {
                        std::memcpy(res_buf.data() + 4 + i * 16, &vec[i].first, 8);
                        std::memcpy(res_buf.data() + 4 + i * 16 + 8, &vec[i].second, 8);
                    }
                    send_response(NXPCommand::RANGE, std::string(res_buf.begin(), res_buf.end()));
                    break;
                }
                case NXPCommand::SNAPSHOT: {
                    db_.snapshot();
                    send_response(NXPCommand::SNAPSHOT, "OK");
                    break;
                }
                case NXPCommand::STATS: {
                    std::string stats_json = db_.get_metrics_json();
                    send_response(NXPCommand::STATS, stats_json);
                    break;
                }
                default:
                    send_response(NXPCommand::NXP_ERROR, "UNKNOWN_CMD");
                    break;
            }
        } catch (const std::exception& e) {
            send_response(NXPCommand::NXP_ERROR, e.what());
        }
    }

    void send_response(NXPCommand cmd, const std::string& payload) {
        auto self(shared_from_this());
        
        response_header_.magic = NXP_MAGIC;
        response_header_.version = NXP_VERSION;
        response_header_.command = static_cast<uint8_t>(cmd);
        response_header_.payload_size = static_cast<uint32_t>(payload.size());
        
        response_data_ = payload; // Keep data alive

        std::vector<asio::const_buffer> buffers;
        buffers.push_back(asio::buffer(&response_header_, sizeof(NXPHeader)));
        buffers.push_back(asio::buffer(response_data_));

        asio::async_write(socket_, buffers,
            [this, self](asio::error_code ec, std::size_t /*length*/) {
                if (!ec) {
                    do_read_header(); // Next command
                }
            });
    }

    tcp::socket socket_;
    DB& db_;
    NXPHeader header_;
    std::vector<char> payload_;
    
    NXPHeader response_header_;
    std::string response_data_;
};

class NXPServer {
public:
    NXPServer(asio::io_context& io_context, short port, DB& db)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), db_(db) {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(
            [this](asio::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::make_shared<NXPSession>(std::move(socket), db_)->start();
                }
                do_accept();
            });
    }

    tcp::acceptor acceptor_;
    DB& db_;
};

int main(int argc, char* argv[]) {
    velodb::print_banner();
    
    std::string data_dir = "./data";
    uint16_t port = 9000;
    
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--data-dir" && i + 1 < argc) data_dir = argv[++i];
        else if (std::string(argv[i]) == "--port" && i + 1 < argc) port = static_cast<uint16_t>(std::stoi(argv[++i]));
    }

    try {
        DB db(data_dir);
        asio::io_context io_context;
        NXPServer server(io_context, port, db);
        
        std::cout << "VeloDB (Nexus Protocol) server listening on port " << port << std::endl;
        
        // Multi-threaded runner
        std::vector<std::thread> threads;
        for (std::size_t i = 0; i < std::thread::hardware_concurrency(); ++i) {
            threads.emplace_back([&io_context]() { io_context.run(); });
        }
        for (auto& t : threads) t.join();

    } catch (const std::exception& e) {
        std::cerr << "Server Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

