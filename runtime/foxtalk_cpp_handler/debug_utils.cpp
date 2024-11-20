//
// Created by lexi on 10/11/24.
//

#include "debug_utils.h"

#include <iostream>
#include <format>
#include <cmath>

void dbg_dump_buffer_region(uint8_t *buffer, size_t start, size_t length) {
    std::cout << "      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F        0 1 2 3 4 5 6 7 8 9 A B C D E F";
    auto n_rows = static_cast<size_t>(std::ceil(static_cast<double>(length) / 16.0));

    for (auto row = 0; row < n_rows; row++) {
        std::cout << std::endl << std::format("0x{:02x} ", row * 16);
        for (auto column = 0; column < 16; column++) {
            auto offset = row * 16 + column;
            if (offset > length) {
                std::cout << " - ";
            } else {
                std::cout << std::format("{:02x} ", buffer[start + offset]);
            }
        }

        std::cout << "      ";

        for (auto column = 0; column < 16; column++) {
            auto offset = row * 16 + column;
            if (offset > length) {
                std::cout << " -";
            } else {
                if (buffer[start + offset] >= 0x20 && buffer[start + offset] <= 0x7E) {
                    std::cout << std::format("{: >2c}", buffer[start + offset]);
                } else {
                    std::cout << " .";
                }
            }
        }
    }

    std::cout << std::endl;
}