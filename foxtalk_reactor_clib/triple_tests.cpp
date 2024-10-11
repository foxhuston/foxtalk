//
// Created by fox on 10/7/24.
//

#include <string>

#include "gtest/gtest.h"

#include "foxtalk_triple.h"

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

TEST_F(TripleTests, TripleSubjectAccessor) {
    Triple nWrite = {
            TripleNoun { "lexi"s },
            TripleNoun { "is a"s },
            TripleNoun { "husky"s }
    };

    auto subj = nWrite.get_subject<std::string>();

    EXPECT_EQ(subj, "lexi"s);
}

TEST_F(TripleTests, TriplePredicateAccessor) {
    Triple nWrite = {
            TripleNoun { "lexi"s },
            TripleNoun { "is a"s },
            TripleNoun { "husky"s }
    };

    auto pred = nWrite.get_predicate<std::string>();

    EXPECT_EQ(pred, "is a"s);
}

TEST_F(TripleTests, TripleObjectAccessor) {
    Triple nWrite = {
            TripleNoun { 0ul },
            TripleNoun { 1ul },
            TripleNoun { 42ul }
    };

    auto subj = nWrite.get_object<uint64_t>();

    EXPECT_EQ(subj, 42ul);
}

TEST_F(TripleTests, TripleSubjectAccessorWrongType) {
    Triple nWrite = {
            TripleNoun { "lexi"s },
            TripleNoun { "is a"s },
            TripleNoun { "husky"s }
    };

    auto subj = nWrite.get_subject<uint64_t>();
    EXPECT_EQ(subj, std::nullopt);
}

TEST_F(TripleTests, TriplePredicateAccessorWrongType) {
    Triple nWrite = {
            TripleNoun { "lexi"s },
            TripleNoun { "is a"s },
            TripleNoun { "husky"s }
    };

    auto pred = nWrite.get_predicate<void *>();
    EXPECT_EQ(pred, std::nullopt);
}

TEST_F(TripleTests, TripleObjectAccessorWrongType) {
    Triple nWrite = {
            TripleNoun { 0ul },
            TripleNoun { 1ul },
            TripleNoun { 42ul }
    };

    auto subj = nWrite.get_object<int64_t>();
    EXPECT_EQ(subj, std::nullopt);
}
