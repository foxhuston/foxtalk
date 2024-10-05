
#include <assert.h>
#include <string.h>
#include "include/database.h"

//constexpr size_t initial_db_size = 1;
constexpr size_t initial_db_size = 20000000; // 20 MTuples

Database *mkNewDatabase() {
    auto tuple_mem = (Tuple *)malloc(sizeof(Tuple) * initial_db_size);
    auto cptr_mem = (RefCountedCPtrWithFree *) malloc(sizeof(RefCountedCPtrWithFree) * initial_db_size);

    memset(tuple_mem, 0, sizeof(Tuple) * initial_db_size);
    memset(cptr_mem, 0, sizeof(RefCountedCPtrWithFree) * initial_db_size);

    auto db = (Database *)malloc(sizeof(Database));
    db->cptr_count = 0;
    db->cptr_alloc_size = initial_db_size;
    db->cptrs = cptr_mem;

    db->tuple_count = 0;
    db->alloc_size = initial_db_size;
    db->tuples = tuple_mem;

    return db;
}

void freeDatabase(Database * db) {
    free(db->tuples);
    free(db);
}

bool tuple_noun_eq(TupleNoun a, TupleNoun b) {
    if(a.type != b.type) return false;

    switch (a.type) {
        case Query:
            return true;
        case Pair:
            exit(100);
        case Sym:
            return a.data.symbol == b.data.symbol;
        case CPtr:
            return a.data.cptr.data == b.data.cptr.data
                && a.data.cptr.free_fn == b.data.cptr.free_fn;
        case U64:
            return a.data.u64 == b.data.u64;
        case I64:
            return a.data.i64 == b.data.i64;
    }
}

void maybe_add_counted_cptr(Database *db, TupleNoun noun) {
    if(noun.type == CPtr) {
        // Is this already in the db?
        for(size_t i = 0; i < db->cptr_count; i++) {
            if(db->cptrs[i].data == noun.data.cptr.data) {
                db->cptrs[i].ref_count++;
                return;
            }
        }

        db->cptrs[db->cptr_count].ref_count = 1;
        db->cptrs[db->cptr_count].data = noun.data.cptr.data;
        db->cptrs[db->cptr_count].free_fn = noun.data.cptr.free_fn;

        db->cptr_count++;
    }
}
void maybe_dec_or_remove_counted_cptr(Database *db, TupleNoun noun) {
    if(noun.type == CPtr) {
        // Is this already in the db?
        for (size_t i = 0; i < db->cptr_count; i++) {
            if (db->cptrs[i].data == noun.data.cptr.data) {
                db->cptrs[i].ref_count--;
                if(db->cptrs[i].ref_count == 0) {
                    // Free the CPtr object
                    db->cptrs[i].free_fn(db->cptrs[i].data);
                    memset(db->cptrs + i, 0xAE, sizeof(RefCountedCPtrWithFree));
                }

                assert(db->cptrs[i].ref_count >= 0);
                return;
            }
        }
    }
}

Tuple* db_addTuple(Database *db, TupleNoun subject, TupleNoun predicate, TupleNoun object) {
    // Do we already have this tuple?
    size_t count = 0;
    auto res = db_query(db, subject, predicate, object, &count);
    if(count == 1) {
        auto out = res->tuple;
        free_db_query_results(res);
        return out;
    }

    assert(count < 1);

    // It's new!
    maybe_add_counted_cptr(db, subject);
    maybe_add_counted_cptr(db, predicate);
    maybe_add_counted_cptr(db, object);

    if((db->tuple_count + 1) >= db->alloc_size) {
        auto new_db_alloc_size = db->alloc_size * 2;
        auto new_tuple_mem = (Tuple *)malloc(sizeof(Tuple) * new_db_alloc_size);

        memset(new_tuple_mem, 0, sizeof(Tuple) * new_db_alloc_size);

        size_t new_tuple_count = 0;
        for(size_t i = 0; i < db->tuple_count; i++) {
            if(!db->tuples[i].is_deleted) {
                memcpy(new_tuple_mem + new_tuple_count, db->tuples + i, sizeof(Tuple));
                new_tuple_count++;
            }

        }

        memset(db->tuples, 0xAE, sizeof(Tuple) * db->alloc_size);
        free(db->tuples);

        db->tuples = new_tuple_mem;
        db->alloc_size = new_db_alloc_size;
    }

    auto new_tuple_idx = db->tuple_count;

    db->tuples[new_tuple_idx].subject = subject;
    db->tuples[new_tuple_idx].predicate = predicate;
    db->tuples[new_tuple_idx].object = object;

    db->tuple_count++;
    return db->tuples + new_tuple_idx;
}

void db_removeTuple(Database* db, TupleNoun subject, TupleNoun predicate, TupleNoun object) {
    size_t results_count = 0;
    auto results = db_query(db, subject, predicate, object, &results_count);
    if(results_count > 0) {
        assert(results_count == 1);

        maybe_dec_or_remove_counted_cptr(db, subject);
        maybe_dec_or_remove_counted_cptr(db, predicate);
        maybe_dec_or_remove_counted_cptr(db, object);

        results->tuple->is_deleted = 1;
    }
}

TupleResult *add_tuple_result(TupleResult* to, Tuple* tuple) {
    auto new_res = (TupleResult*)malloc(sizeof(TupleResult));
    new_res->tuple = tuple;
    new_res->next = nullptr;

    if(to == nullptr) {
        return new_res;
    }

    to->next = new_res;
    return new_res;
}

void free_db_query_results(TupleResult *to) {
    if(to == nullptr) return;

    TupleResult *next;

    do {
        next = to->next;
        free(to);
        to = next;
    } while(to != nullptr);
}

TupleResult* db_query(Database* db, TupleNoun subject, TupleNoun predicate, TupleNoun object, size_t *results_count) {
    TupleResult *out = nullptr;
    TupleResult *current = nullptr;
    size_t count = 0;

    for(size_t i = 0; i < db->tuple_count; i++) {
        auto db_tup = db->tuples + i;
        if(db_tup->is_deleted) { continue; }

        if(subject.type   != Query && !tuple_noun_eq(db_tup->subject, subject)) { continue; }
        if(predicate.type != Query && !tuple_noun_eq(db_tup->predicate, predicate)) { continue; }
        if(object.type    != Query && !tuple_noun_eq(db_tup->object, object)) { continue; }

        count++;
        current = add_tuple_result(out, db_tup);
        if(out == nullptr) {
            out = current;
        }
    }

    *results_count = count;
    return out;
}
