//
// Created by lexi on 10/10/24.
//

#include "vendor/kuzu.hpp"

using namespace kuzu::main;


#include "foxtalk_db.h"

#include "gtest/gtest.h"

using namespace std::literals;

class DbTests : public ::testing::Test {
};

std::string test_string = "testing!";

TEST_F(DbTests, YellowBrickRoad) {


    SystemConfig config;
    auto db = foxtalk_db(config);

    uint64_t u64 = 123894;
    int64_t i64 = 123894;
    auto str_n = TripleNoun { std::string("lexi")};
    auto u64_n = TripleNoun { u64 };
    auto i64_n = TripleNoun { i64 };

    void* hello_world_addr = &test_string;

    auto cptr_n = TripleNoun { hello_world_addr };

    auto trip = Triple {
        TripleNoun { std::string("lexi") },
        TripleNoun { std::string("is a") },
        TripleNoun { std::string("husky") }
    };

    db.store_triple(&trip);

    auto query = Triple {
        TripleNoun {  },
        TripleNoun { std::string("is a") },
        TripleNoun { std::string("husky") },
    };

    auto results = db.get_triples(&query);

    EXPECT_EQ(results[0].get_subject<std::string>(), "lexi");
}
