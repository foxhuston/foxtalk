//
// Created by fox on 10/22/24.
//

#include <vector>
#include <string>

#include "gtest/gtest.h"

#include "foxtalk_handler.hpp"

using namespace std::literals;

class HandlerTests : public ::testing::Test {
};


class EmptyHandler : public Handler {
public:
    size_t match_count = 0;

    void handle(const std::vector<Tuple> &queryResults) override {
        match_count = queryResults.size();
    }

    bool matches(const Tuple &n) override {
        return true;
    }
};

TEST_F(HandlerTests, EmptyHandlerClaimsNothing) {
    std::vector<Tuple> queryResults{
            {
                    Tuple{{
                                  TupleNoun{"lexi"s},
                                  TupleNoun{"is a"s},
                                  TupleNoun{"husky"s}
                          }},
                    Tuple{{
                                  TupleNoun{"lexi"s},
                                  TupleNoun{"is"s},
                                  TupleNoun{"cool"s}

                          }},
            }};

    EmptyHandler e{};
    e.handle(queryResults);

    ASSERT_EQ(e.claims.size(), 0);
    ASSERT_EQ(e.match_count, 2);
}

TEST_F(HandlerTests, EmptyHandlerFfiWorks) {
    std::vector<Tuple> queryResults{
            {
                    Tuple{{
                                  TupleNoun{"lexi"s},
                                  TupleNoun{"is a"s},
                                  TupleNoun{"husky"s}
                          }},
                    Tuple{{
                                  TupleNoun{"lexi"s},
                                  TupleNoun{"is"s},
                                  TupleNoun{"cool"s}

                          }},
            }};

    EmptyHandler e {};

    uint8_t buffer[256] {};
//    std::cout << "======== INITIAL BUFFER ========" << std::endl;
//    dbg_dump_buffer_region(buffer, 0, 256);

//    std::cout << "======== TEST WRITE TO BUFFER ========" << std::endl;
    write_vec_to_buffer(buffer, 0, queryResults);
//    dbg_dump_buffer_region(buffer, 0, 256);

    e.ffi_handle(buffer);

//    std::cout << "======== POST HANDLER BUFFER ========" << std::endl;
    auto [res, bytes_read] = read_vec_from_buffer<Tuple>(buffer, 0);

    ASSERT_EQ(res.size(), 0);
    ASSERT_EQ(e.match_count, 2);
}


class HuskyHandler : public Handler {
public:
    bool matches(const Tuple &n) override {
        return n.at<std::string>(1).has_value() && n.at<std::string>(1).value() == "is a" &&
            n.at<std::string>(2).has_value() && n.at<std::string>(2).value() == "husky";
    }

    void handle(const std::vector<Tuple> &queryResults) override {
        for (auto& i: queryResults)
        {
            auto subj = i.at<std::string>(0).value();
            claim(Tuple {std::vector{TupleNoun(subj) , TupleNoun("is"), TupleNoun("cool") } });
        }
    }
};

FOXTALK_FFI_HANDLER_REG(HuskyHandler);


TEST_F(HandlerTests, HuskyHandlerClaimsCoolness) {
    std::vector<Tuple> queryResults{
                {
                    Tuple{{
                        TupleNoun{"lexi"s},
                        TupleNoun{"is a"s},
                        TupleNoun{"husky"s}
                    }}
}};

    HuskyHandler e{};
    e.handle(queryResults);

    ASSERT_EQ(e.claims.size(), 1);
    ASSERT_EQ(e.claims.at(0).at<std::string>(0), "lexi");
    ASSERT_EQ(e.claims.at(0).at<std::string>(1), "is");
    ASSERT_EQ(e.claims.at(0).at<std::string>(2), "cool");
}

