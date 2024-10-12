//
// Created by fox on 10/7/24.
//
#define CTRACK_DISABLE_EXECUTION_POLICY
#include <string>

#include "gtest/gtest.h"

#include "foxtalk_triple.h"
#include "foxtalk_handler.h"
#include "reactor.h"
#include "cypher_gen.h"

using namespace std::literals;

class HandlerTests : public ::testing::Test {};

FOXTALK_INIT {
    FOXTALK_REGISTER_HANDLE_QUERY({{}, {"is a"s}, {"husky"s}});
}

FOXTALK_HANDLE {
    if(auto maybe_t = FOXTALK_GET_NEXT_QUERY_RESULT(); maybe_t.has_value()) {
        auto who = maybe_t->get_subject<std::string>();
        claim({ { who.value() }, { "is"s }, { "cool"s } }, handlerEnvironment);
    }
}

FOXTALK_FREE_TUPLE { }
FOXTALK_TEARDOWN { }

TEST_F(HandlerTests, WhoIsCool) {
    kuzu::main::SystemConfig db_config { };
    auto db = std::make_shared<kuzu::main::Database>(":memory:", db_config);

    Reactor r { db };

    // Create Handler
    r.claim({ {"test"s}, {"is a"s}, {"handler"s} });

    r.claim({ {"test"s}, {"has init"s}, { reinterpret_cast<void *>(init) }});
    r.claim({ {"test"s}, {"has handle"s}, { reinterpret_cast<void *>(handle) }});
    r.claim({ {"test"s}, {"has free tuple"s}, { reinterpret_cast<void *>(free_tuple) }});
    r.claim({ {"test"s}, {"has teardown"s}, { reinterpret_cast<void *>(teardown) }});
    r.claim({ {"test"s}, {"has ipc_buffer"s}, { static_cast<void *>(_foxtalk_ipc_triple_buffer) }});

    // Add <lexi, is a, husky>
    r.claim({ {"lexi"s}, {"is a"s}, {"husky"s} });

    // Tick
    r.tick(); // initialize
    r.tick(); // handle

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
