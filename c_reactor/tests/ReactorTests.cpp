//
// Created by fox on 10/2/24.
//


#define BOOST_TEST_DYN_LINK

//#include <boost/test/included/unit_test.hpp>
#include <algorithm>
#include <boost/test/unit_test.hpp>
#include "Tuple.h"
#include "Reactor.h"
#include "DynamicHandler.h"

using namespace foxtalk;


static size_t handler_result_count;

struct TestHandler : Handler {
    Tuple *get_query() const override {
        return new Tuple{TupleNoun::mkSymbol("lexi"), TupleNoun::mkSymbol("is a"), TupleNoun::mkSymbol("husky")};
    }

    void handle_results(TupleVec tv, std::function<void(Tuple)> claim) const override {
        handler_result_count++;
    }
};

BOOST_AUTO_TEST_SUITE(REACTOR_TESTS)

    BOOST_AUTO_TEST_CASE(ReactorAddTupleTest) {
        Reactor reactor;
        reactor.claim(Tuple{TupleNoun::mkSymbol("lexi"), TupleNoun::mkSymbol("is a"), TupleNoun::mkSymbol("husky")});
    }

    BOOST_AUTO_TEST_CASE(ReactorAddHandlerTest) {
        Reactor reactor;
        TestHandler h {};
        reactor.add_handler(&h);
    }

    BOOST_AUTO_TEST_CASE(ReactorCallsHandlerTest) {
        Reactor reactor;
        TestHandler h {};

        reactor.claim(Tuple{TupleNoun::mkSymbol("lexi"), TupleNoun::mkSymbol("is a"), TupleNoun::mkSymbol("husky")});
        reactor.add_handler(&h);

        reactor.tick();

        BOOST_ASSERT(handler_result_count == 1);
    }

    BOOST_AUTO_TEST_CASE(ReactorCallsExternalHandler) {
        Reactor reactor;
        DynamicHandler dh("../../tests/libreactor_test_handler.so");

        reactor.claim(Tuple{TupleNoun::mkSymbol("lexi"), TupleNoun::mkSymbol("is a"), TupleNoun::mkSymbol("husky")});
        reactor.add_handler(&dh);

        reactor.tick();

        auto tuples = reactor.get_db().get_tuples();

        auto expected = Tuple{TupleNoun::mkSymbol("lexi"), TupleNoun::mkSymbol("is"), TupleNoun::mkSymbol("super cool")};
        auto res = std::find(tuples.begin(), tuples.end(), expected);
        BOOST_ASSERT(res != std::end(tuples));

        auto expected2 = Tuple{TupleNoun::mkSymbol("lexi"), TupleNoun::mkSymbol("is a"), TupleNoun::mkSymbol("puppy")};
        auto res2 = std::find(tuples.begin(), tuples.end(), expected2);
        BOOST_ASSERT(res2 != std::end(tuples));
    }

BOOST_AUTO_TEST_SUITE_END()