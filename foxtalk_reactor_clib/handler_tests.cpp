//
// Created by fox on 10/7/24.
//
#define CTRACK_DISABLE_EXECUTION_POLICY
#include <string>

#include "gtest/gtest.h"

#include "foxtalk_triple.h"
#include "foxtalk_handler.h"
#include "reactor.h"

using namespace std::literals;

class HandlerTests : public ::testing::Test {
    friend class Reactor;
};




void non_aggregating_init() {
    Triple q {
            TripleNoun::query(),
            TripleNoun { "is a"s },
            TripleNoun { "husky"s },
    };

    write_to_ipc_buffer(q);
}

void non_aggregating_handle() {
    if(auto maybe_t = getNextQueryResult(); maybe_t.has_value()) {
        auto who = maybe_t->get_subject<std::string>();
        claim({
           { who.value() },
           { "is"s },
           { "cool"s }
        });
    }
}

void non_aggregating_free_tuple() { }
void non_aggregating_teardown() { }



TEST_F(HandlerTests, WhoIsCool) {
    kuzu::main::SystemConfig db_config { };
    auto db = std::make_shared<kuzu::main::Database>(":memory:", db_config);

    Reactor r { db };

    // Create Handler
    r.claim({ {"test"s}, {"is a"s}, {"handler"s} });

    r.claim({ {"test"s}, {"has init"s}, { reinterpret_cast<void *>(non_aggregating_init) }});
    r.claim({ {"test"s}, {"has handle"s}, { reinterpret_cast<void *>(non_aggregating_handle) }});
    r.claim({ {"test"s}, {"has free tuple"s}, { reinterpret_cast<void *>(non_aggregating_free_tuple) }});
    r.claim({ {"test"s}, {"has teardown"s}, { reinterpret_cast<void *>(non_aggregating_teardown) }});

    // Add <lexi, is a, husky>
    r.claim({ {"lexi"s}, {"is a"s}, {"husky"s} });

    // Tick
    r.tick();

    // Assert <lexi, is, cool>
    // d.query(...)
    auto query = R"(MATCH(a:Noun)-[b:Predicate{string_data: "is"}]->(c:Noun{string_data:"cool"}) RETURN a.type, a.string_data;)";
    kuzu::main::Connection conn (db.get());

    auto q = conn.query(query);
    auto results = q.get();
    if(!results->isSuccess()) {
        std::cerr << "Query failed with: " << results->getErrorMessage() << std::endl;
        FAIL();
    }

    ASSERT_EQ(results->getNumTuples(), 1);
    auto result = results->getNext();
    ASSERT_EQ(result->getValue(0)->strVal, "Symbol");
    ASSERT_EQ(result->getValue(1)->strVal, "lexi");

}
