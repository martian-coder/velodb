/*
 * VeloDB: High-Performance Asynchronous Storage Engine
 * Copyright (c) 2026 Amit Nilajkar <amit.nilajkar@gmail.com>
 * 
 * This software and the associated Nexus Protocol (NXP) are the 
 * intellectual property of Amit Nilajkar.
 * Licensed under the MIT License. See LICENSE file for details.
 */

#ifndef VELODB_BANNER_H
#define VELODB_BANNER_H

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

namespace velodb {

inline void setup_terminal() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif
}

inline void print_banner() {
    setup_terminal();

    const std::string cyan = "\x1b[36m";
    const std::string magenta = "\x1b[35m";
    const std::string yellow = "\x1b[33m";
    const std::string reset = "\x1b[0m";
    const std::string bold = "\x1b[1m";

    std::vector<std::string> lines = {
        "  __      __  _        _____  ____  ",
        "  \\ \\    / / | |      |  __ \\|  _ \\ ",
        "   \\ \\  / /__| | ___  | |  | | |_) |",
        "    \\ \\/ / _ \\ |/ _ \\ | |  | |  _ < ",
        "     \\  /  __/ | (_) || |__| | |_) |",
        "      \\/ \\___|_|\\___/ |_____/|____/ ",
        "                                    ",
        "      HIGH-INTEGRITY STORAGE ENGINE  "
    };

    std::cout << "\n";
    for (size_t i = 0; i < lines.size(); ++i) {
        // High-impact Golden-Yellow for the whole banner
        std::cout << bold << yellow << lines[i] << reset << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "\n    " << bold << yellow << ">> Built by Amit Nilajkar <<" << reset << "\n";
    std::cout << "    " << yellow << "Nexus Protocol (NXP) v1.0 Active" << reset << "\n\n";
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

} // namespace velodb

#endif // VELODB_BANNER_H
