/*
 * VeloDB: High-Performance Asynchronous Storage Engine
 * Copyright (c) 2026 Amit Nilajkar <amit.nilajkar@gmail.com>
 * 
 * This software and the associated Nexus Protocol (NXP) are the 
 * intellectual property of Amit Nilajkar.
 * Licensed under the MIT License. See LICENSE file for details.
 */

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <cassert>
#include <atomic>
#include "velodb/db.h"

int main() {
    std::cout << "Starting Multi-Threaded Stress Test..." << std::endl;
    
    // Clear the test dictionary
    static const char* TEST_DIR = "./stress_test_data";
    // We ideally should clean up the directory, but for a simple test we can just use a unique one
    
    velodb::DB db(TEST_DIR);

    const int NUM_WRITERS = 50;
    const int NUM_READERS = 50;
    const int OPS_PER_THREAD = 50;

    std::atomic<int> writes_done{0};
    std::atomic<int> reads_done{0};

    auto start_time = std::chrono::high_resolution_clock::now();

    auto writer_func = [&](int thread_id) {
        try {
            std::mt19937_64 rng(thread_id + 100);
            for (int i = 0; i < OPS_PER_THREAD; ++i) {
                uint64_t key = rng() % 100000;
                uint64_t val = rng();
                db.put(key, val);
                writes_done.fetch_add(1, std::memory_order_relaxed);
            }
        } catch (const std::exception& e) {
            std::cerr << "Writer " << thread_id << " threw exception: " << e.what() << std::endl;
            std::terminate();
        } catch (...) {
            std::cerr << "Writer " << thread_id << " threw unknown exception" << std::endl;
            std::terminate();
        }
    };

    auto reader_func = [&](int thread_id) {
        try {
            std::mt19937_64 rng(thread_id + 200);
            for (int i = 0; i < OPS_PER_THREAD; ++i) {
                uint64_t key = rng() % 100000;
                uint64_t val;
                db.get(key, val);
                reads_done.fetch_add(1, std::memory_order_relaxed);
            }
        } catch (const std::exception& e) {
            std::cerr << "Reader " << thread_id << " threw exception: " << e.what() << std::endl;
            std::terminate();
        } catch (...) {
            std::cerr << "Reader " << thread_id << " threw unknown exception" << std::endl;
            std::terminate();
        }
    };

    std::vector<std::thread> threads;

    // Start all threads simultaneously
    for (int i = 0; i < NUM_WRITERS; ++i) {
        threads.emplace_back(writer_func, i);
    }
    for (int i = 0; i < NUM_READERS; ++i) {
        threads.emplace_back(reader_func, i);
    }

    // Wait for all threads to finish
    for (auto& t : threads) {
        t.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end_time - start_time;

    assert(writes_done == NUM_WRITERS * OPS_PER_THREAD);
    assert(reads_done == NUM_READERS * OPS_PER_THREAD);

    int total_ops = writes_done + reads_done;
    double ops_per_sec = total_ops / diff.count();

    std::cout << "Stress test completed successfully!" << std::endl;
    std::cout << "Total time: " << diff.count() << " seconds" << std::endl;
    std::cout << "Throughput: " << ops_per_sec << " operations/second" << std::endl;

    return 0;
}

