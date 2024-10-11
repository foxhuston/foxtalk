//
// Created by fox on 10/7/24.
//

#include <string>

#include "gtest/gtest.h"

#include "foxtalk_handler.h"

using namespace std::literals;

class TripleTests : public ::testing::Test {
};

TEST_F(TripleTests, TripleNounQueryRoundTrip) {
    uint8_t buffer[64] {};

    auto nWrite = TripleNoun();
    nWrite.write_to_buffer(buffer, 0);

    auto [nRead, read_bytes] = TripleNoun::read_from_buffer(buffer, 0);

    EXPECT_EQ(nRead, nWrite);
}

TEST_F(TripleTests, TripleNounSymbolRoundTrip) {
    uint8_t buffer[64] {};
    constexpr size_t start_position = 0x5;
    dbg_dump_buffer_region(buffer, 0, 64);
    std::cout << "===== ORIG =====================================================================" << std::endl;

    auto s = "Hello, World!"s;
    auto nWrite = TripleNoun(s);

    size_t bytes_written = nWrite.write_to_buffer(buffer, start_position);
    //                       type byte       + string length marker               + actual string length
    EXPECT_EQ(bytes_written, sizeof(uint8_t) + (sizeof(foxtalk_size_t) / sizeof(uint8_t)) + s.length());

    dbg_dump_buffer_region(buffer, 0, 64);
    std::cout << "===== POST WRITE ===============================================================" << std::endl;

    auto [nRead, read_bytes] = TripleNoun::read_from_buffer(buffer, start_position);

    EXPECT_EQ(nRead, nWrite);
}

TEST_F(TripleTests, TripleNounCptrRoundTrip) {
    uint8_t buffer[64] {};

    auto nWrite = TripleNoun(&buffer);
    nWrite.write_to_buffer(buffer, 0);

    auto [nRead, read_bytes] = TripleNoun::read_from_buffer(buffer, 0);

    EXPECT_EQ(nRead, nWrite);
}

TEST_F(TripleTests, TripleNounU64RoundTrip) {
    uint8_t buffer[64] {};

    auto nWrite = TripleNoun(4242ul);
    nWrite.write_to_buffer(buffer, 0);

    auto [nRead, read_bytes] = TripleNoun::read_from_buffer(buffer, 0);

    EXPECT_EQ(nRead, nWrite);
}

TEST_F(TripleTests, TripleNounI64RoundTrip) {
    uint8_t buffer[64] {};

    auto nWrite = TripleNoun(2424l);
    nWrite.write_to_buffer(buffer, 0);

    auto [nRead, read_bytes] = TripleNoun::read_from_buffer(buffer, 0);

    EXPECT_EQ(nRead, nWrite);
}

TEST_F(TripleTests, TripleRoundTrip) {
    uint8_t buffer[64] {};
    std::cout << "===== ORIG =====================================================================" << std::endl;
    dbg_dump_buffer_region(buffer, 0, 64);

    Triple nWrite = {
        TripleNoun { "lexi"s },
        TripleNoun { "is a"s },
        TripleNoun { "husky"s }
    };

    nWrite.write_to_buffer(buffer, 0);
    std::cout << "===== POST WRITE ===============================================================" << std::endl;
    dbg_dump_buffer_region(buffer, 0, 64);

    std::cout << "===== WILL READ ================================================================" << std::endl;
    auto [nRead, read_bytes] = Triple::read_from_buffer(buffer, 0);

    EXPECT_EQ(nRead, nWrite);
}
