#include <assert.h>
#include <string.h>
#include "gtest/gtest.h"
#include "stdio.h"

#include "database/database.h"

class DbTests : public ::testing::Test {

};

TEST_F(DbTests, TestDbAdd) {
    auto db = mkNewDatabase();

    TupleNoun subj = { .type = TupleNoun::Type::U64, .data = { .u64 = 4242 } };
    TupleNoun pred = { .type = TupleNoun::Type::U64, .data = { .u64 = 5353 } };
    TupleNoun obj  = { .type = TupleNoun::Type::U64, .data = { .u64 = 6464 } };

    auto new_tuple = addTuple(db, subj, pred, obj);

    assert(new_tuple->subject.type == TupleNoun::Type::U64 && new_tuple->subject.data.u64 == 4242);
    assert(new_tuple->predicate.type == TupleNoun::Type::U64 && new_tuple->predicate.data.u64 == 5353);
    assert(new_tuple->object.type == TupleNoun::Type::U64 && new_tuple->object.data.u64 == 6464);

    freeDatabase(db);
}
//
//
//Tuple* test_db_fn_add(Database *db) {
//    TupleNoun subj = { .type = U64, .data.u64 = 4242 };
//    TupleNoun pred = { .type = U64, .data.u64 = 5353 };
//    TupleNoun obj  = { .type = U64, .data.u64 = 6464 };
//
//    return addTuple(db, subj, pred, obj);
//}
//
//void test_db_add_from_fn_call() {
//    auto db = mkNewDatabase();
//    auto new_tuple = test_db_fn_add(db);
//
//    assert(new_tuple->subject.type == U64 && new_tuple->subject.data.u64 == 4242);
//    assert(new_tuple->predicate.type == U64 && new_tuple->predicate.data.u64 == 5353);
//    assert(new_tuple->object.type == U64 && new_tuple->object.data.u64 == 6464);
//
//    freeDatabase(db);
//}
//
////void test_db_add_multiple_tuples() {
////    auto db = mkNewDatabase();
////    TupleNoun subj = { .type = U64, .data.u64 = 4242 };
////    TupleNoun pred = { .type = U64, .data.u64 = 5353 };
////    TupleNoun obj  = { .type = U64, .data.u64 = 6464 };
////
////    auto first_tuple = addTuple(db, subj, pred, obj);
////    assert(db->alloc_size == 2);
////    addTuple(db, subj, pred, obj); // Should resize
////    assert(db->alloc_size == 4);
////    addTuple(db, subj, pred, obj); // Should resize again
////    assert(db->alloc_size == 4);
////
////    assert(first_tuple->subject.type == U64 && first_tuple->subject.data.u64 == 4242);
////    assert(first_tuple->predicate.type == U64 && first_tuple->predicate.data.u64 == 5353);
////    assert(first_tuple->object.type == U64 && first_tuple->object.data.u64 == 6464);
////}
//
//static int has_freed_str = 0;
//
//void str_free(void *data) {
//    memset(data, 0xAE, 10);
//    has_freed_str = 1;
//    free(data);
//}
//
//void remove_should_mark_removed() {
//    auto db = mkNewDatabase();
//
//    TupleNoun subj = { .type = U64, .data.u64 = 4242 };
//    TupleNoun pred = { .type = U64, .data.u64 = 5353 };
//    TupleNoun obj  = { .type = U64, .data.u64 = 6464 };
//
//    auto new_tuple = addTuple(db, subj, pred, obj);
//    removeTuple(db, new_tuple);
//
//    assert(new_tuple->is_deleted == 1);
//}
//
//void should_free_cptrs_when_no_longer_needed() {
//    has_freed_str = 0;
//    auto db = mkNewDatabase();
//
//    auto str = (char *)malloc(sizeof(char) * 10);
//    memset(str, 0, 10);
//    strcpy(str, "Hello!");
//
//    TupleNoun subj = { .type = CPtr, .data.cptr = { .data = str, .free_fn = str_free } };
//    TupleNoun pred = { .type = U64, .data.u64 = 5353 };
//    TupleNoun obj  = { .type = U64, .data.u64 = 6464 };
//
//    auto the_tuple = addTuple(db, subj, pred, obj);
//    removeTuple(db, the_tuple);
//
//    assert(has_freed_str == 1);
//}
//
//void should_free_cptrs_when_no_longer_needed_2() {
//    has_freed_str = 0;
//    auto db = mkNewDatabase();
//
//    auto str = (char *)malloc(sizeof(char) * 10);
//    memset(str, 0, 10);
//    strcpy(str, "Hello!");
//
//    TupleNoun subj = { .type = CPtr, .data.cptr = { .data = str, .free_fn = str_free } };
//    TupleNoun pred = { .type = U64, .data.u64 = 5353 };
//    TupleNoun obj  = { .type = U64, .data.u64 = 6464 };
//
//    auto the_tuple = addTuple(db, subj, pred, obj);
//
//    addTuple(db, pred, obj, the_tuple->subject);
//    removeTuple(db, the_tuple);
//
//    assert(has_freed_str == 0);
//}
//
//void should_free_cptrs_when_no_longer_needed_3() {
//    has_freed_str = 0;
//    auto db = mkNewDatabase();
//
//    auto str = (char *)malloc(sizeof(char) * 10);
//    memset(str, 0, 10);
//    strcpy(str, "Hello!");
//
//    TupleNoun subj = { .type = CPtr, .data.cptr = { .data = str, .free_fn = str_free } };
//    TupleNoun pred = { .type = U64, .data.u64 = 5353 };
//    TupleNoun obj  = { .type = U64, .data.u64 = 6464 };
//
//    auto the_tuple = addTuple(db, subj, pred, obj);
//
//    auto new_tuple = addTuple(db, pred, obj, the_tuple->subject);
//    removeTuple(db, the_tuple);
//    assert(has_freed_str == 0);
//
//    removeTuple(db, new_tuple);
//    assert(has_freed_str == 1);
//}
//
//void it_should_find_tuples() {
//    auto db = mkNewDatabase();
//
//    TupleNoun subj = { .type = U64, .data.u64 = 4242 };
//    TupleNoun pred = { .type = U64, .data.u64 = 5353 };
//    TupleNoun obj  = { .type = U64, .data.u64 = 6464 };
//
//    auto new_tuple = addTuple(db, subj, pred, obj);
//
//    size_t results_count;
//
//    TupleNoun queryNoun = { .type = Query };
//    auto results = query(db, queryNoun, pred, obj, &results_count);
//
//    assert(results_count == 1);
//    assert(results[0].tuple == new_tuple);
//    assert(results[0].tuple->subject.data.u64 == 4242);
//
//    free_tuple_results(results);
//}
//
//
//int main() {
//    test_db_add();
//    test_db_add_from_fn_call();
//    remove_should_mark_removed();
//    should_free_cptrs_when_no_longer_needed();
//    should_free_cptrs_when_no_longer_needed_2();
//    should_free_cptrs_when_no_longer_needed_3();
//    it_should_find_tuples();
//
//    // TODO:
////    test_db_add_multiple_tuples();
//    return 0;
//}
