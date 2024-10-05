#include <cstring>
#include "gtest/gtest.h"

#include "database.h"

class DbTests : public ::testing::Test {

};

TEST_F(DbTests, TestDbAdd) {
    auto db = mkNewDatabase();

    TupleNoun subj = { .type = TupleNoun::Type::U64, .data = { .u64 = 4242 } };
    TupleNoun pred = { .type = TupleNoun::Type::U64, .data = { .u64 = 5353 } };
    TupleNoun obj  = { .type = TupleNoun::Type::U64, .data = { .u64 = 6464 } };

    auto new_tuple = db_addTuple(db, subj, pred, obj);

    EXPECT_EQ(new_tuple->subject.type, TupleNoun::Type::U64);
    EXPECT_EQ(new_tuple->subject.data.u64, 4242);
    EXPECT_EQ(new_tuple->predicate.type, TupleNoun::Type::U64);
    EXPECT_EQ(new_tuple->predicate.data.u64, 5353);
    EXPECT_EQ(new_tuple->object.type, TupleNoun::Type::U64);
    EXPECT_EQ(new_tuple->object.data.u64, 6464);

    freeDatabase(db);
}


Tuple* test_db_fn_add(Database *db) {
    TupleNoun subj = { .type = TupleNoun::Type::U64, .data = { .u64 = 4242 } };
    TupleNoun pred = { .type = TupleNoun::Type::U64, .data = { .u64 = 5353 } };
    TupleNoun obj  = { .type = TupleNoun::Type::U64, .data = { .u64 = 6464 } };

    return db_addTuple(db, subj, pred, obj);
}

TEST_F(DbTests, TestDbAddFromFnCall) {
    auto db = mkNewDatabase();
    auto new_tuple = test_db_fn_add(db);

    EXPECT_EQ(new_tuple->subject.type, TupleNoun::Type::U64);
    EXPECT_EQ(new_tuple->subject.data.u64, 4242);
    EXPECT_EQ(new_tuple->predicate.type, TupleNoun::Type::U64);
    EXPECT_EQ(new_tuple->predicate.data.u64, 5353);
    EXPECT_EQ(new_tuple->object.type, TupleNoun::Type::U64);
    EXPECT_EQ(new_tuple->object.data.u64, 6464);

    freeDatabase(db);
}

TEST_F(DbTests, InsertionIsIdempotent) {
    auto db = mkNewDatabase();

    TupleNoun subj = { .type = TupleNoun::Type::U64, .data = { .u64 = 4242 } };
    TupleNoun pred = { .type = TupleNoun::Type::U64, .data = { .u64 = 5353 } };
    TupleNoun obj  = { .type = TupleNoun::Type::U64, .data = { .u64 = 6464 } };

    auto t1 = db_addTuple(db, subj, pred, obj);
    auto t2 = db_addTuple(db, subj, pred, obj);

    EXPECT_EQ(t1, t2);
    EXPECT_EQ(db->tuple_count, 1);

    TupleNoun newObj = { .type = TupleNoun::Type::I64, .data = { .i64 = 0 } };
    db_addTuple(db, subj, pred, newObj);

    EXPECT_EQ(db->tuple_count, 2);

    freeDatabase(db);
}

TEST_F(DbTests, DISABLED_TestDbGrowsWithMultipleInserts) {
    auto db = mkNewDatabase();
    TupleNoun subj = { .type = TupleNoun::Type::U64, .data = { .u64 = 4242 } };
    TupleNoun pred = { .type = TupleNoun::Type::U64, .data = { .u64 = 5353 } };
    TupleNoun obj  = { .type = TupleNoun::Type::U64, .data = { .u64 = 6464 } };

    auto first_tuple = db_addTuple(db, subj, pred, obj);
    EXPECT_EQ(db->alloc_size, 2);
    db_addTuple(db, subj, pred, obj); // Should resize
    EXPECT_EQ(db->alloc_size, 4);
    db_addTuple(db, subj, pred, obj); // Should resize again
    EXPECT_EQ(db->alloc_size, 4);

    EXPECT_EQ(first_tuple->subject.type, TupleNoun::Type::U64);
    EXPECT_EQ(first_tuple->subject.data.u64, 4242);
    EXPECT_EQ(first_tuple->predicate.type, TupleNoun::Type::U64);
    EXPECT_EQ(first_tuple->predicate.data.u64, 5353);
    EXPECT_EQ(first_tuple->object.type, TupleNoun::Type::U64);
    EXPECT_EQ(first_tuple->object.data.u64, 6464);
}

static int has_freed_str = 0;

void str_free(void *data) {
    memset(data, 0xAE, 10);
    has_freed_str = 1;
    free(data);
}

TEST_F(DbTests, RemoveShouldMarkRemoved) {
    auto db = mkNewDatabase();

    TupleNoun subj = { .type = TupleNoun::Type::U64, .data = { .u64 = 4242 } };
    TupleNoun pred = { .type = TupleNoun::Type::U64, .data = { .u64 = 5353 } };
    TupleNoun obj  = { .type = TupleNoun::Type::U64, .data = { .u64 = 6464 } };

    auto new_tuple = db_addTuple(db, subj, pred, obj);
    db_removeTuple(db, new_tuple->subject, new_tuple->predicate, new_tuple->object);

    EXPECT_TRUE(new_tuple->is_deleted == 1);
}

TEST_F(DbTests, ShouldFreeCptrsWhenNoLongerNeeded) {
    has_freed_str = 0;
    auto db = mkNewDatabase();

    auto str = (char *)malloc(sizeof(char) * 10);
    memset(str, 0, 10);
    strcpy(str, "Hello!");

    TupleNoun subj = { .type = TupleNoun::Type::CPtr, .data = { .cptr = { .data = str, .free_fn = str_free } } };
    TupleNoun pred = { .type = TupleNoun::Type::U64, .data = { .u64 = 5353 } };
    TupleNoun obj  = { .type = TupleNoun::Type::U64, .data = { .u64 = 6464 } };

    auto new_tuple = db_addTuple(db, subj, pred, obj);
    db_removeTuple(db, new_tuple->subject, new_tuple->predicate, new_tuple->object);

    EXPECT_TRUE(has_freed_str == 1);
}

TEST_F(DbTests, ShouldFreeCptrsWhenNoLongerNeeded2) {
    has_freed_str = 0;
    auto db = mkNewDatabase();

    auto str = (char *)malloc(sizeof(char) * 10);
    memset(str, 0, 10);
    strcpy(str, "Hello!");

    TupleNoun subj = { .type = TupleNoun::Type::CPtr, .data = { .cptr = { .data = str, .free_fn = str_free } } };
    TupleNoun pred = { .type = TupleNoun::Type::U64, .data = { .u64 = 5353 } };
    TupleNoun obj  = { .type = TupleNoun::Type::U64, .data = { .u64 = 6464 } };

    auto the_tuple = db_addTuple(db, subj, pred, obj);

    db_addTuple(db, pred, obj, the_tuple->subject);
    db_removeTuple(db, subj, pred, obj);

    EXPECT_TRUE(has_freed_str == 0);
}

TEST_F(DbTests, ShouldFreeCptrsWhenNoLongerNeeded3) {
    has_freed_str = 0;
    auto db = mkNewDatabase();

    auto str = (char *)malloc(sizeof(char) * 10);
    memset(str, 0, 10);
    strcpy(str, "Hello!");

    TupleNoun subj = { .type = TupleNoun::Type::CPtr, .data = { .cptr = { .data = str, .free_fn = str_free } } };
    TupleNoun pred = { .type = TupleNoun::Type::U64, .data = { .u64 = 5353 } };
    TupleNoun obj  = { .type = TupleNoun::Type::U64, .data = { .u64 = 6464 } };

    auto the_tuple = db_addTuple(db, subj, pred, obj);

    auto new_tuple = db_addTuple(db, pred, obj, the_tuple->subject);
    db_removeTuple(db, subj, pred, obj);
    EXPECT_TRUE(has_freed_str == 0);

    db_removeTuple(db, new_tuple->subject, new_tuple->predicate, new_tuple->object);
    EXPECT_TRUE(has_freed_str == 1);
}

TEST_F(DbTests, ShouldFindTuples) {
    auto db = mkNewDatabase();

    TupleNoun subj = { .type = TupleNoun::Type::U64, .data = { .u64 = 4242 } };
    TupleNoun pred = { .type = TupleNoun::Type::U64, .data = { .u64 = 5353 } };
    TupleNoun obj  = { .type = TupleNoun::Type::U64, .data = { .u64 = 6464 } };

    auto new_tuple = db_addTuple(db, subj, pred, obj);

    size_t results_count;

    auto results = db_query(db, queryNoun, pred, obj, &results_count);

    EXPECT_TRUE(results_count == 1);
    EXPECT_EQ(results[0].tuple, new_tuple);
    EXPECT_EQ(results[0].tuple->subject.data.u64, 4242);

    free_db_query_results(results);
}

TEST_F(DbTests, TestDbWithSymbols) {
    auto db = mkNewDatabase();

    TupleNoun subj = { .type = TupleNoun::Type::Sym, .data = { .symbol = intern("lexi") } };
    TupleNoun pred = { .type = TupleNoun::Type::Sym, .data = { .symbol = intern("is a") } };
    TupleNoun obj  = { .type = TupleNoun::Type::Sym, .data = { .symbol = intern("husky") } };

    auto new_tuple = db_addTuple(db, subj, pred, obj);

    EXPECT_EQ(new_tuple->subject.type, TupleNoun::Type::Sym);
    EXPECT_EQ(new_tuple->subject.data.symbol, intern("lexi"));
    EXPECT_EQ(new_tuple->predicate.type, TupleNoun::Type::Sym);
    EXPECT_EQ(new_tuple->predicate.data.symbol, intern("is a"));
    EXPECT_EQ(new_tuple->object.type, TupleNoun::Type::Sym);
    EXPECT_EQ(new_tuple->object.data.symbol, intern("husky"));

    EXPECT_STREQ(new_tuple->object.data.symbol->str, "husky");

    freeDatabase(db);
}
