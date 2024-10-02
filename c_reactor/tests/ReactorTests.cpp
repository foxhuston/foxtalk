//
// Created by fox on 10/2/24.
//


#define BOOST_TEST_DYN_LINK

//#include <boost/test/included/unit_test.hpp>
#include <boost/test/unit_test.hpp>
#include "Tuple.h"
#include "Reactor.h"

using namespace foxtalk;

Tuple *test_get_query() {
    return new Tuple{TupleNoun::mkSymbol("lexi"), TupleNoun::mkSymbol("is a"), TupleNoun::mkSymbol("husky")};
}

static size_t handler_result_count = 0;

void test_handle_results(TupleVec query_results) {
    handler_result_count = query_results.size();
}

BOOST_AUTO_TEST_SUITE(REACTOR_TESTS)

    BOOST_AUTO_TEST_CASE(ReactorAddTupleTest) {
        Reactor reactor;
        reactor.claim(Tuple{TupleNoun::mkSymbol("lexi"), TupleNoun::mkSymbol("is a"), TupleNoun::mkSymbol("husky")});
    }

    BOOST_AUTO_TEST_CASE(ReactorAddHandlerTest) {
        Reactor reactor;
        Handler h(test_get_query, test_handle_results);
        reactor.add_handler(h);
    }

    BOOST_AUTO_TEST_CASE(ReactorCallsHandlerTest) {
        Reactor reactor;
        Handler h(test_get_query, test_handle_results);

        reactor.claim(Tuple{TupleNoun::mkSymbol("lexi"), TupleNoun::mkSymbol("is a"), TupleNoun::mkSymbol("husky")});
        reactor.add_handler(h);

        reactor.tick();

        BOOST_ASSERT(handler_result_count == 1);
    }

    BOOST_AUTO_TEST_CASE(ReactorCallsExternalHandler) {
        Reactor reactor;
        // TODO
        BOOST_ASSERT(false);
    }

BOOST_AUTO_TEST_SUITE_END()