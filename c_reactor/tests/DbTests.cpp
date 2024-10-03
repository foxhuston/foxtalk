//
// Created by fox on 10/2/24.
//


#define BOOST_TEST_DYN_LINK

//don't need to repeat this define in more than one cpp file
#define BOOST_TEST_MAIN

//#include <boost/test/included/unit_test.hpp>
#include <boost/test/unit_test.hpp>
#include "Tuple.h"
#include "Db.h"

using namespace foxtalk;

BOOST_AUTO_TEST_SUITE(DB_TESTS)

    BOOST_AUTO_TEST_CASE(DbAddsTupleTest) {
        Db db;
        db.add_tuple(Tuple::mk(TupleNoun::mkSymbol("lexi"), TupleNoun::mkSymbol("is a"), TupleNoun::mkSymbol("husky")));
        auto count = db.get_tuples().size();
        BOOST_ASSERT(count == 1);
    }

    BOOST_AUTO_TEST_CASE(DbRemovesTupleTest) {
        Db db;
        db.add_tuple(Tuple::mk(TupleNoun::mkSymbol("lexi"), TupleNoun::mkSymbol("is a"), TupleNoun::mkSymbol("husky")));

        auto count = db.get_tuples().size();
        BOOST_ASSERT(count == 1);

        db.remove_tuple(Tuple::mk(TupleNoun::mkSymbol("lexi"), TupleNoun::mkSymbol("is a"), TupleNoun::mkSymbol("husky")));

        auto post_delete_count = db.get_tuples().size();
        BOOST_ASSERT(post_delete_count == 0);
    }

BOOST_AUTO_TEST_SUITE_END()