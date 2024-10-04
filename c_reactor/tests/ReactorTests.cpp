//
// Created by fox on 10/2/24.
//

#define BOOST_TEST_DYN_LINK

// #include <boost/test/included/unit_test.hpp>
#include "Reactor.h"
#include "SharedObjectHandler.h"
#include "Tuple.h"
#include <algorithm>
#include <boost/test/unit_test.hpp>

using namespace foxtalk;

// TODO: This is really not ergonomic.
static size_t handler_result_count;

struct TestHandler : Handler {
  Tuple *get_query() const override {
    return Tuple::mk(mkSymbol("lexi"), mkSymbol("is a"), mkSymbol("husky"));
  }

  void handle_results(ReactorVec<const Tuple *>::type tv,
                      std::function<void(const Tuple *)> claim) const override {
    handler_result_count++;
  }
};

BOOST_AUTO_TEST_SUITE(REACTOR_TESTS)

BOOST_AUTO_TEST_CASE(TupleNounHashTest) {
  auto hasher = boost::hash<const TupleNoun *>();
  auto h1 = hasher(mkSymbol("lexi"));
  auto h2 = hasher(mkSymbol("lexi"));

  std::cout << "h1 = " << h1 << "; h2 = " << h2 << std::endl;

  BOOST_ASSERT(h1 == h2);
}

BOOST_AUTO_TEST_CASE(ReactorAddTupleTest) {
  Reactor reactor;
  reactor.claim(mkSymbol("lexi"), mkSymbol("is a"), mkSymbol("husky"));
}

BOOST_AUTO_TEST_CASE(ReactorDoubleAddTupleTest) {
  Reactor reactor;
  reactor.claim(mkSymbol("lexi"), mkSymbol("is a"), mkSymbol("husky"));
  reactor.claim(mkSymbol("lexi"), mkSymbol("is a"), mkSymbol("husky"));

  auto size = reactor.get_db().size();
  std::cout << "db size: " << size << std::endl;

  BOOST_ASSERT(size == 1);
}

BOOST_AUTO_TEST_CASE(ReactorAddHandlerTest) {
  Reactor reactor;
  TestHandler h{};
  reactor.add_handler(&h);
}

BOOST_AUTO_TEST_CASE(ReactorCallsHandlerTest) {
  handler_result_count = 0;

  Reactor reactor;
  TestHandler h{};

  reactor.claim(mkSymbol("lexi"), mkSymbol("is a"), mkSymbol("husky"));
  reactor.add_handler(&h);

  reactor.tick();

  BOOST_ASSERT(handler_result_count == 1);
}

BOOST_AUTO_TEST_CASE(ReactorCallsHandlerOnlyOnceTest) {
  handler_result_count = 0;

  Reactor reactor;
  TestHandler h{};

  reactor.claim(mkSymbol("lexi"), mkSymbol("is a"), mkSymbol("husky"));
  reactor.add_handler(&h);

  reactor.tick();
  reactor.tick();
  reactor.tick();
  reactor.tick();

  BOOST_ASSERT(handler_result_count == 1);
}

BOOST_AUTO_TEST_CASE(ReactorWillCallHandlerAgainTest) {
  handler_result_count = 0;

  Reactor reactor;
  TestHandler h{};

  reactor.claim(mkSymbol("lexi"), mkSymbol("is a"), mkSymbol("husky"));
  reactor.add_handler(&h);

  reactor.tick();
  reactor.tick();
  reactor.tick();
  reactor.tick();

  BOOST_ASSERT(handler_result_count == 1);

  reactor.remove(mkSymbol("lexi"), mkSymbol("is a"), mkSymbol("husky"));
  reactor.claim(mkSymbol("lexi"), mkSymbol("is a"), mkSymbol("husky"));

  reactor.tick();
  reactor.tick();
  reactor.tick();
  reactor.tick();

  BOOST_ASSERT(handler_result_count == 2);
}

struct MaxHandler : Handler {
  Tuple *get_query() const override {
    return Tuple::mk(mkQuery(), mkSymbol("has size"), mkQuery());
  }

  void handle_results(ReactorVec<const Tuple *>::type tv,
                      std::function<void(const Tuple *)> claim) const override {
    const TupleNoun *max_subj = nullptr;
    uint64_t max_size = 0;

    for (auto t : tv) {
      if (t->getObject()->is_u64() && t->getObject()->data.u64 > max_size) {
        max_size = t->getObject()->data.u64;
        max_subj = t->getSubject();
      }
    }

    if (max_subj != nullptr) {
      claim(Tuple::mk(max_subj, mkSymbol("is"), mkSymbol("the biggest!")));
    }
  }
};

BOOST_AUTO_TEST_CASE(ReactorWillRemoveGeneratedTuplesTest) {
  Reactor reactor;
  MaxHandler h{};

  reactor.claim(mkSymbol("ball a"), mkSymbol("has size"), mkU64(1));
  reactor.claim(mkSymbol("ball b"), mkSymbol("has size"), mkU64(2));
  reactor.claim(mkSymbol("ball c"), mkSymbol("has size"), mkU64(3));
  reactor.claim(mkSymbol("ball d"), mkSymbol("has size"), mkU64(1000));

  reactor.add_handler(&h);

  ///// FIRST TICK /////////////////////////////////////////////////////////////

  reactor.tick();

  auto results =
      reactor.query(mkQuery(), mkSymbol("is"), mkSymbol("the biggest!"));

  std::cout << "Results contains \"ball d is the biggest?\"" << std::endl;
  BOOST_ASSERT(results.contains(
      Tuple::mk(mkSymbol("ball d"), mkSymbol("is"), mkSymbol("the biggest!"))));

  ///// ERASE & SECOND TICK ////////////////////////////////////////////////////
  reactor.remove(mkSymbol("ball d"), mkSymbol("has size"), mkU64(1000));
  reactor.tick();
  reactor.tick();

  ///// POST-ERASE/TICK RESULTS ////////////////////////////////////////////////
  auto post_delete_results =
      reactor.query(mkQuery(), mkSymbol("is"), mkSymbol("the biggest!"));

  std::cout << "Post delete results:" << std::endl;
  for (auto t : post_delete_results) {
    std::cout << "   " << *t << std::endl;
  }

  auto inner_db_results = reactor.get_db().get_tuples().contains(
      Tuple::mk(mkSymbol("ball d"), mkSymbol("has size"), mkU64(1000)));

  std::cout << "Post remove db NOT contains \"ball d is the biggest?\""
            << std::endl;
  BOOST_ASSERT(!inner_db_results);

  std::cout << "Post delete results NOT contains \"ball d is the biggest?\""
            << std::endl;
  auto contains_ball_d_is_biggest = post_delete_results.contains(
      Tuple::mk(mkSymbol("ball d"), mkSymbol("is"), mkSymbol("the biggest!")));

  BOOST_ASSERT(!contains_ball_d_is_biggest);

  std::cout << "Post delete results contains \"ball c is the biggest?\""
            << std::endl;

  auto contains_ball_c_is_biggest = post_delete_results.contains(
      Tuple::mk(mkSymbol("ball c"), mkSymbol("is"), mkSymbol("the biggest!")));

  BOOST_ASSERT(contains_ball_c_is_biggest);
}

BOOST_AUTO_TEST_CASE(ReactorCallsExternalHandler) {
  Reactor reactor;
  SharedObjectHandler dh("../../tests/libreactor_test_handler.so");

  reactor.claim(mkSymbol("lexi"), mkSymbol("is a"), mkSymbol("husky"));
  reactor.add_handler(&dh);

  reactor.tick();

  auto tuples = reactor.get_db().get_tuples();

  auto expected =
      Tuple::mk(mkSymbol("lexi"), mkSymbol("is"), mkSymbol("super cool"));
  BOOST_ASSERT(tuples.contains(expected));

  auto expected2 =
      Tuple::mk(mkSymbol("lexi"), mkSymbol("is an"), mkSymbol("awesome puppy"));

  BOOST_ASSERT(tuples.contains(expected2));
}

struct TestClaimingHandler : Handler {
  Tuple *get_query() const override {
    return Tuple::mk(mkSymbol("lexi"), mkSymbol("is a"), mkSymbol("husky"));
  }

  void handle_results(ReactorVec<const Tuple *>::type tv,
                      std::function<void(const Tuple *)> claim) const override {
    for (auto t : tv) {
      claim(Tuple::mk(t->getSubject(), mkSymbol("is"), mkSymbol("cool")));

      claim(Tuple::mk(mkSymbol("fox"), mkSymbol("loves"), t->getSubject()));
    }
  }
};

BOOST_AUTO_TEST_CASE(ReactorGeneratedTuplesAreTransitivelyRemoved) {
  Reactor reactor;
  TestClaimingHandler th;

  auto originalClaim =
      Tuple::mk(mkSymbol("lexi"), mkSymbol("is a"), mkSymbol("husky"));
  reactor.claim(originalClaim);
  reactor.add_handler(&th);

  auto expectedA =
      Tuple::mk(mkSymbol("lexi"), mkSymbol("is"), mkSymbol("cool"));
  auto expectedB =
      Tuple::mk(mkSymbol("fox"), mkSymbol("loves"), mkSymbol("lexi"));

  reactor.tick();

  auto tuples = reactor.get_db().get_tuples();
  BOOST_ASSERT(tuples.contains(originalClaim));
  BOOST_ASSERT(tuples.contains(expectedA));
  BOOST_ASSERT(tuples.contains(expectedB));

  reactor.remove(mkSymbol("lexi"), mkSymbol("is a"), mkSymbol("husky"));
  reactor.tick();
  reactor.tick();

  auto tuples2 = reactor.get_db().get_tuples();
  BOOST_ASSERT(!tuples2.contains(originalClaim));
  BOOST_ASSERT(!tuples2.contains(expectedA));
  BOOST_ASSERT(!tuples2.contains(expectedB));
}

BOOST_AUTO_TEST_CASE(
    ReactorGeneratedTuplesAreTransitivelyRemovedByHandlerRemoval) {
  Reactor reactor;
  TestClaimingHandler th;

  auto originalClaim =
      Tuple::mk(mkSymbol("lexi"), mkSymbol("is a"), mkSymbol("husky"));

  reactor.claim(originalClaim);
  reactor.add_handler(&th);

  auto expectedA =
      Tuple::mk(mkSymbol("lexi"), mkSymbol("is"), mkSymbol("cool"));
  auto expectedB =
      Tuple::mk(mkSymbol("fox"), mkSymbol("loves"), mkSymbol("lexi"));

  reactor.tick();

  auto tuples = reactor.get_db().get_tuples();
  BOOST_ASSERT(tuples.contains(originalClaim));
  BOOST_ASSERT(tuples.contains(expectedA));
  BOOST_ASSERT(tuples.contains(expectedB));

  reactor.remove_handler(&th);
  reactor.tick();
  reactor.tick();

  auto tuples2 = reactor.get_db().get_tuples();
  BOOST_ASSERT(tuples2.contains(originalClaim));
  BOOST_ASSERT(!tuples2.contains(expectedA));
  BOOST_ASSERT(!tuples2.contains(expectedB));
}

BOOST_AUTO_TEST_CASE(ReactorHandlerDeleteAndReaddWorks) {
  Reactor reactor;
  TestClaimingHandler th;

  auto originalClaim =
      Tuple::mk(mkSymbol("lexi"), mkSymbol("is a"), mkSymbol("husky"));

  reactor.claim(originalClaim);
  reactor.add_handler(&th);

  auto expectedA =
      Tuple::mk(mkSymbol("lexi"), mkSymbol("is"), mkSymbol("cool"));
  auto expectedB =
      Tuple::mk(mkSymbol("fox"), mkSymbol("loves"), mkSymbol("lexi"));

  reactor.tick();

  auto tuples = reactor.get_db().get_tuples();
  BOOST_ASSERT(tuples.contains(originalClaim));
  BOOST_ASSERT(tuples.contains(expectedA));
  BOOST_ASSERT(tuples.contains(expectedB));

  reactor.remove_handler(&th);
  reactor.tick();

  auto tuples2 = reactor.get_db().get_tuples();
  BOOST_ASSERT(tuples2.contains(originalClaim));
  BOOST_ASSERT(!tuples2.contains(expectedA));
  BOOST_ASSERT(!tuples2.contains(expectedB));

  reactor.add_handler(&th);
  reactor.tick();

  auto tuples3 = reactor.get_db().get_tuples();
  BOOST_ASSERT(tuples3.contains(originalClaim));
  BOOST_ASSERT(tuples3.contains(expectedA));
  BOOST_ASSERT(tuples3.contains(expectedB));
}

BOOST_AUTO_TEST_CASE(DeleteHandlerGeneratedItem) {
  Reactor r{};
  r.add_handler(new TestClaimingHandler{});

  r.claim(mkSymbol("lexi"), mkSymbol("is a"), mkSymbol("husky"));
  r.tick();
  r.tick();

  auto res = r.query(mkQuery(), mkSymbol("is a"), mkSymbol("husky"));
  r.remove(res);
  r.tick();
  r.tick();
}

BOOST_AUTO_TEST_SUITE_END()