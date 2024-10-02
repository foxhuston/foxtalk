//
// Created by fox on 10/2/24.
//


#define BOOST_TEST_DYN_LINK

//don't need to repeat this define in more than one cpp file
#define BOOST_TEST_MAIN

//#include <boost/test/included/unit_test.hpp>
#include <boost/test/unit_test.hpp>
#include "Db.h"


BOOST_AUTO_TEST_SUITE(DB_TESTS)

BOOST_AUTO_TEST_CASE(DbTest) {
    foxtalk::Db db;
}

BOOST_AUTO_TEST_SUITE_END()