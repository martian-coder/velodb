/*
 * VeloDB: High-Performance Asynchronous Storage Engine
 * Copyright (c) 2026 Amit Nilajkar <amit.nilajkar@gmail.com>
 * 
 * This software and the associated Nexus Protocol (NXP) are the 
 * intellectual property of Amit Nilajkar.
 * Licensed under the MIT License. See LICENSE file for details.
 */

#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>
#include "velodb/db.h"

void test_put_get() {
    std::cout << "Running PutGet..." << std::endl;
    std::filesystem::remove_all("./testdata");
    velodb::DB db("./testdata");
    db.put(1, 100);
    db.put(2, 200);
    uint64_t v;
    assert(db.get(1, v));
    assert(v == 100);
    assert(db.get(2, v));
    assert(v == 200);
    assert(!db.get(3, v));
    std::cout << "PutGet passed!" << std::endl;
}

void test_range() {
    std::cout << "Running Range..." << std::endl;
    velodb::DB db("./testdata");
    for (uint64_t i = 10; i < 20; ++i) db.put(i, i*10);
    auto vec = db.range(12, 16);
    assert(vec.size() == 5);
    for (size_t i = 0; i < vec.size(); ++i) {
        assert(vec[i].first == 12 + i);
        assert(vec[i].second == (12 + i)*10);
    }
    std::cout << "Range passed!" << std::endl;
}

void test_recovery() {
    std::cout << "Running Recovery (Corruption Test)..." << std::endl;
    std::string path = "./testdata_recovery";
    std::filesystem::remove_all(path);
    
    {
        velodb::DB db(path);
        db.put(10, 1000);
        db.put(20, 2000);
        // Force flush via destructor
    }

    // Intentionally corrupt the log by appending garbage
    std::string log_p = path + "/log.bin";
    std::ofstream out(log_p, std::ios::binary | std::ios::app);
    out.write("CORRUPTION_GARBAGE", 18);
    out.close();

    // Re-open and verify recovery
    {
        velodb::DB db(path);
        uint64_t v;
        assert(db.get(10, v));
        assert(v == 1000);
        assert(db.get(20, v));
        assert(v == 2000);
        
        // Ensure no crash on range scan
        auto res = db.range(0, 100);
        assert(res.size() == 2);
    }
    std::cout << "Recovery passed!" << std::endl;
}

int main() {
    test_put_get();
    test_range();
    test_recovery();
    std::cout << "All tests passed!" << std::endl;
    return 0;
}

