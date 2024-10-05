//
// Created by fox on 10/5/24.
//

#include "gtest/gtest.h"
#include "reactor.h"

class ReactorTests : public ::testing::Test { };

TEST_F(ReactorTests, MkReactor) {
    auto r = mkReactor();
    freeReactor(r);
}

TupleNoun lexi = { .type = TupleNoun::Type::Sym, .data = { .symbol = intern("lexi") } };
TupleNoun isA = { .type = TupleNoun::Type::Sym, .data = { .symbol = intern("is a") } };
TupleNoun husky  = { .type = TupleNoun::Type::Sym, .data = { .symbol = intern("husky") } };

TEST_F(ReactorTests, ReactorInsertTuple) {
    auto r = mkReactor();

    reactor_addTuple(r, lexi, isA, husky);
    EXPECT_EQ(r->db->tuple_count, 1); // TODO: Holy cow this should not be (in general) visible.

    freeReactor(r);
}

TEST_F(ReactorTests, ReactorRemoveTuple) {
    auto r = mkReactor();

    auto t = reactor_addTuple(r, lexi, isA, husky);
    reactor_removeTuple(r, t);
    EXPECT_EQ(r->db->tuple_count, 1);
    EXPECT_EQ(r->db->tuples->is_deleted, 1);

    freeReactor(r);
}

static size_t counting_handle_fn_called_times = 0;
void counting_handle_fn(TupleResult *results) {
    counting_handle_fn_called_times++;
}

TEST_F(ReactorTests, ReactorAddHandler) {
    auto r = mkReactor();

    EXPECT_EQ(r->handler_count, 0);
    reactor_addHandler(r, queryNoun, isA, husky, counting_handle_fn);
    EXPECT_EQ(r->handler_count, 1);

    freeReactor(r);
}

TEST_F(ReactorTests, ReactorRemoveHandler) {
    auto r = mkReactor();

    EXPECT_EQ(r->handler_count, 0);
    auto h= reactor_addHandler(r, queryNoun, isA, husky, counting_handle_fn);
    EXPECT_EQ(r->handler_count, 1);
    reactor_removeHandler(r, h);
    EXPECT_EQ(r->handler_count, 1);
    EXPECT_TRUE(r->handlers->is_deleted);

    freeReactor(r);
}
