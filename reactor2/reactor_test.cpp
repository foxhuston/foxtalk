//
// Created by fox on 10/5/24.
//

#include "gtest/gtest.h"
#include "reactor.h"
#include "TestTupleNouns.h"

class ReactorTests : public ::testing::Test { };

TEST_F(ReactorTests, MkReactor) {
    auto r = mkReactor();
    freeReactor(r);
}

TEST_F(ReactorTests, ReactorInsertTuple) {
    auto r = mkReactor();

    reactor_addTuple(r, lexi, isA, husky);
    EXPECT_EQ(r->db->tuple_count, 1); // TODO: Holy cow this should not be (in general) visible.

    freeReactor(r);
}

TEST_F(ReactorTests, ReactorRemoveTuple) {
    auto r = mkReactor();

    auto t = reactor_addTuple(r, lexi, isA, husky);
    reactor_removeTuple(r, lexi, isA, husky);
    EXPECT_EQ(r->db->tuple_count, 1);
    EXPECT_EQ(r->db->tuples->is_deleted, 1);

    freeReactor(r);
}

static size_t counting_handle_fn_called_times = 0;
void counting_handle_fn(Reactor *r, TupleResult *results) {
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

TEST_F(ReactorTests, ReactorCallsHandlerOnExistingTuples) {
    counting_handle_fn_called_times = 0;
    auto r = mkReactor();

    reactor_addTuple(r, lexi, isA, husky);
    reactor_tick(r);

    reactor_addHandler(r, queryNoun, isA, husky, counting_handle_fn);
    reactor_tick(r);

    EXPECT_EQ(counting_handle_fn_called_times, 1);

    freeReactor(r);
}

TEST_F(ReactorTests, ReactorCallsHandlerOnNewTuples) {
    counting_handle_fn_called_times = 0;
    auto r = mkReactor();

    reactor_addHandler(r, queryNoun, isA, husky, counting_handle_fn);
    reactor_tick(r);

    reactor_addTuple(r, lexi, isA, husky);
    reactor_tick(r);

    EXPECT_EQ(counting_handle_fn_called_times, 1);

    freeReactor(r);
}

TEST_F(ReactorTests, ReactorCallsHandlerOnlyOnceOnSameTuples) {
    counting_handle_fn_called_times = 0;
    auto r = mkReactor();

    reactor_addHandler(r, queryNoun, isA, husky, counting_handle_fn);
    reactor_tick(r);

    reactor_addTuple(r, lexi, isA, husky);
    reactor_tick(r);

    EXPECT_EQ(counting_handle_fn_called_times, 1);

    reactor_addTuple(r, lexi, isA, husky);
    reactor_tick(r);

    EXPECT_EQ(counting_handle_fn_called_times, 1);

    freeReactor(r);
}

void tuple_generating_handler_fn(Reactor* r, TupleResult* query_results) {
    for(auto res = query_results; res != nullptr; res = res->next) {
        reactor_addTuple(r, res->tuple->subject, is, cool);
    }
}

TEST_F(ReactorTests, ReactorHandlerGeneratesTuples) {
    auto r = mkReactor();

    reactor_addHandler(r, queryNoun, isA, husky, tuple_generating_handler_fn);
    reactor_tick(r);

    reactor_addTuple(r, lexi, isA, husky);
    reactor_tick(r);

    size_t results_count;
    auto res = db_query(r->db, queryNoun, is, cool, &results_count);
    EXPECT_EQ(results_count, 1);
    if(results_count > 0) {
        EXPECT_EQ(res->tuple->subject.type, TupleNoun::Type::Sym);
        EXPECT_EQ(res->tuple->subject.data.symbol, intern("lexi"));
    }

    freeReactor(r);
}

TEST_F(ReactorTests, ReactorHandlerRemovesGeneratesTuplesWhenOriginatingTupleIsRemoved) {
    auto r = mkReactor();

    reactor_addHandler(r, queryNoun, isA, husky, tuple_generating_handler_fn);
    reactor_tick(r);

    reactor_addTuple(r, lexi, isA, husky);
    reactor_tick(r);

    size_t results_count;
    auto res = db_query(r->db, queryNoun, is, cool, &results_count);
    EXPECT_EQ(results_count, 1);
    if(results_count > 0) {
        EXPECT_EQ(res->tuple->subject.type, TupleNoun::Type::Sym);
        EXPECT_EQ(res->tuple->subject.data.symbol, intern("lexi"));

        reactor_removeTuple(r, lexi, isA, husky);
        reactor_tick(r);

        auto remove_res = db_query(r->db, queryNoun, is, cool, &results_count);

        EXPECT_EQ(results_count, 0);
        free_db_query_results(remove_res);
    }

    free_db_query_results(res);

    freeReactor(r);
}

TEST_F(ReactorTests, ReactorHandlerRemovesGeneratesTuplesWhenOriginatingHandlerIsRemoved) {
    auto r = mkReactor();

    auto hid = reactor_addHandler(r, queryNoun, isA, husky, tuple_generating_handler_fn);
    reactor_tick(r);

    reactor_addTuple(r, lexi, isA, husky);
    reactor_tick(r);

    size_t results_count;
    auto res = db_query(r->db, queryNoun, is, cool, &results_count);
    EXPECT_EQ(results_count, 1);
    if(results_count > 0) {
        EXPECT_EQ(res->tuple->subject.type, TupleNoun::Type::Sym);
        EXPECT_EQ(res->tuple->subject.data.symbol, intern("lexi"));

        reactor_removeHandler(r, hid);
        reactor_tick(r);
        size_t remove_results_count;
        auto remove_res = db_query(r->db, queryNoun, is, cool, &remove_results_count);

        EXPECT_EQ(remove_results_count, 0);
        free_db_query_results(remove_res);
    }

    free_db_query_results(res);

    freeReactor(r);
}
