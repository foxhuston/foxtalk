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
    uint8_t buffer[__foxtalk_ipc_buffer_size] {};

    auto nWrite = TripleNoun();
    nWrite.write_to_buffer(buffer, 0);

    auto [nRead, read_bytes] = TripleNoun::read_from_buffer(buffer, 0);

    EXPECT_EQ(nRead, nWrite);
}

TEST_F(TripleTests, TripleNounSymbolRoundTrip) {
    uint8_t buffer[__foxtalk_ipc_buffer_size] {};

    auto s = "Hello, World!"s;
    auto nWrite = TripleNoun(s);

    nWrite.write_to_buffer(buffer, 0);

    auto [nRead, read_bytes] = TripleNoun::read_from_buffer(buffer, 0);

    EXPECT_EQ(nRead, nWrite);
}

TEST_F(TripleTests, TripleNounCptrRoundTrip) {
    uint8_t buffer[__foxtalk_ipc_buffer_size];

    auto nWrite = TripleNoun(&buffer);
    nWrite.write_to_buffer(buffer, 0);

    auto [nRead, read_bytes] = TripleNoun::read_from_buffer(buffer, 0);

    EXPECT_EQ(nRead, nWrite);
}

TEST_F(TripleTests, TripleNounU64RoundTrip) {
    uint8_t buffer[__foxtalk_ipc_buffer_size];

    auto nWrite = TripleNoun(4242ul);
    nWrite.write_to_buffer(buffer, 0);

    auto [nRead, read_bytes] = TripleNoun::read_from_buffer(buffer, 0);

    EXPECT_EQ(nRead, nWrite);
}

TEST_F(TripleTests, TripleNounI64RoundTrip) {
    uint8_t buffer[__foxtalk_ipc_buffer_size];

    auto nWrite = TripleNoun(2424l);
    nWrite.write_to_buffer(buffer, 0);

    auto [nRead, read_bytes] = TripleNoun::read_from_buffer(buffer, 0);

    EXPECT_EQ(nRead, nWrite);
}
