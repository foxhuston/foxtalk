//
// Created by fox on 10/7/24.
//

#include <string>

#include "gtest/gtest.h"

#include "foxtalk_tuple.h"

using namespace std::literals;

class TupleTests : public ::testing::Test {
};

TEST_F(TupleTests, TripleNounQueryRoundTrip) {
    uint8_t buffer[64]{};

    auto nWrite = TupleNoun();
    nWrite.write_to_buffer(buffer, 0);

    auto [nRead, read_bytes] = TupleNoun::read_from_buffer(buffer, 0);

    EXPECT_EQ(nRead, nWrite);
}

TEST_F(TupleTests, TripleNounSymbolRoundTrip) {
    uint8_t buffer[64]{};
    constexpr size_t start_position = 0x5;
    dbg_dump_buffer_region(buffer, 0, 64);
    std::cout << "===== ORIG =====================================================================" << std::endl;

    auto s = "Hello, World!"s;
    auto nWrite = TupleNoun(s);

    size_t bytes_written = nWrite.write_to_buffer(buffer, start_position);
    //                       type byte       + string length marker               + actual string length
    EXPECT_EQ(bytes_written, sizeof(uint8_t) + (sizeof(foxtalk_size_t) / sizeof(uint8_t)) + s.length());

    dbg_dump_buffer_region(buffer, 0, 64);
    std::cout << "===== POST WRITE ===============================================================" << std::endl;

    auto [nRead, read_bytes] = TupleNoun::read_from_buffer(buffer, start_position);

    EXPECT_EQ(nRead, nWrite);
}

TEST_F(TupleTests, TripleNounCptrRoundTrip) {
    uint8_t buffer[64]{};

    auto nWrite = TupleNoun(&buffer);
    nWrite.write_to_buffer(buffer, 0);

    auto [nRead, read_bytes] = TupleNoun::read_from_buffer(buffer, 0);

    EXPECT_EQ(nRead, nWrite);
}

TEST_F(TupleTests, TripleNounU64RoundTrip) {
    uint8_t buffer[64]{};

    auto nWrite = TupleNoun(4242ul);
    nWrite.write_to_buffer(buffer, 0);

    auto [nRead, read_bytes] = TupleNoun::read_from_buffer(buffer, 0);

    EXPECT_EQ(nRead, nWrite);
}

TEST_F(TupleTests, TripleNounI64RoundTrip) {
    uint8_t buffer[64]{};

    auto nWrite = TupleNoun(2424l);
    nWrite.write_to_buffer(buffer, 0);

    auto [nRead, read_bytes] = TupleNoun::read_from_buffer(buffer, 0);

    EXPECT_EQ(nRead, nWrite);
}

TEST_F(TupleTests, TripleRoundTrip) {
    uint8_t buffer[64]{};
    std::cout << "===== ORIG =====================================================================" << std::endl;
    dbg_dump_buffer_region(buffer, 0, 64);

    Tuple nWrite{{
                         TupleNoun{"lexi"s},
                         TupleNoun{"is a"s},
                         TupleNoun{"husky"s}
                  }};

    nWrite.write_to_buffer(buffer, 0);
    std::cout << "===== POST WRITE ===============================================================" << std::endl;
    dbg_dump_buffer_region(buffer, 0, 64);

    std::cout << "===== WILL READ ================================================================" << std::endl;
    auto [nRead, read_bytes] = Tuple::read_from_buffer(buffer, 0);

    EXPECT_EQ(nRead, nWrite);
}

TEST_F(TupleTests, TripleSubjectAccessor) {
    Tuple nWrite{{
                         TupleNoun{"lexi"s},
                         TupleNoun{"is a"s},
                         TupleNoun{"husky"s}
                  }};

    auto subj = nWrite.at<std::string>(0);

    EXPECT_EQ(subj, "lexi"s);
}

TEST_F(TupleTests, TriplePredicateAccessor) {
    Tuple nWrite{{
                         TupleNoun{"lexi"s},
                         TupleNoun{"is a"s},
                         TupleNoun{"husky"s}
                  }};

    auto pred = nWrite.at<std::string>(1);

    EXPECT_EQ(pred, "is a"s);
}

TEST_F(TupleTests, TripleObjectAccessor) {
    Tuple nWrite{{
                         TupleNoun{0ul},
                         TupleNoun{1ul},
                         TupleNoun{42ul}
                  }};

    if (auto subj = nWrite.at<uint64_t>(2)) {
        EXPECT_EQ(subj, 42ul);
    } else {
        FAIL();
    }
}

TEST_F(TupleTests, TripleSubjectAccessorWrongType) {
    Tuple nWrite{{
                         TupleNoun{"lexi"s},
                         TupleNoun{"is a"s},
                         TupleNoun{"husky"s}
                  }};

    auto subj = nWrite.at<uint64_t>(0);
    EXPECT_EQ(subj, std::nullopt);
}

TEST_F(TupleTests, TriplePredicateAccessorWrongType) {
    Tuple nWrite{{
                         TupleNoun{"lexi"s},
                         TupleNoun{"is a"s},
                         TupleNoun{"husky"s}
                  }};

    auto pred = nWrite.at<void *>(1);
    EXPECT_EQ(pred, std::nullopt);
}

TEST_F(TupleTests, TripleObjectAccessorWrongType) {
    Tuple nWrite{{
                         TupleNoun{0ul},
                         TupleNoun{1ul},
                         TupleNoun{42ul}
                  }};

    if (auto subj = nWrite.at<int64_t>(2)) {
        FAIL();
    }
}

TEST_F(TupleTests, TupleNounVecRoundTrip1) {
    std::vector<TupleNoun> tupleNouns{{
                                              TupleNoun{"lexi"s},
                                              TupleNoun{"is a"s},
                                              TupleNoun{"husky"s}
                                      }};

    uint8_t buffer[256]{};
    write_vec_to_buffer<TupleNoun>(buffer, 0, tupleNouns);
    dbg_dump_buffer_region(buffer, 0, 256);

    auto [res, bytes_read] = read_vec_from_buffer<TupleNoun>(buffer, 0);

    ASSERT_EQ(tupleNouns, res);
}

TEST_F(TupleTests, TupleVecRoundTrip1) {
    std::vector<Tuple> tuples{{
                                      Tuple{{
                                                    TupleNoun{"lexi"s},
                                                    TupleNoun{"is a"s},
                                                    TupleNoun{"husky"s}
                                            }}
                              }};

    uint8_t buffer[256]{};
    write_vec_to_buffer<Tuple>(buffer, 0, tuples);
    dbg_dump_buffer_region(buffer, 0, 256);

    // I would expect: 01 00 00 00 03 00 00 00 ...

    auto [res, bytes_read] = read_vec_from_buffer<Tuple>(buffer, 0);

    ASSERT_EQ(tuples, res);
}

TEST_F(TupleTests, TupleVecRoundTrip2) {
    std::vector<Tuple> tuples{{
                                      Tuple{{
                                                    TupleNoun{"lexi"s},
                                                    TupleNoun{"is a"s},
                                                    TupleNoun{"husky"s}
                                            }},

                                      Tuple{{
                                                    TupleNoun{"lexi"s},
                                                    TupleNoun{"is"s},
                                                    TupleNoun{"cool"s}
                                            }}
                              }};

    uint8_t buffer[256]{};
    write_vec_to_buffer<Tuple>(buffer, 0, tuples);
    dbg_dump_buffer_region(buffer, 0, 256);

    // I would expect: 01 00 00 00 03 00 00 00 ...

    auto [res, bytes_read] = read_vec_from_buffer<Tuple>(buffer, 0);

    ASSERT_EQ(tuples, res);
}