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

    EmptyHandler e {};
    e.handle(queryResults);

    ASSERT_EQ(e.claims.size(), 0);
    ASSERT_EQ(e.match_count, 2);
}
