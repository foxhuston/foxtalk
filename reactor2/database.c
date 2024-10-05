
#include <string.h>
#include "database.h"

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

                return;
            }
        }
    }
}

Tuple* addTuple(Database *db, TupleNoun subject, TupleNoun predicate, TupleNoun object) {
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

void removeTuple(Database* db, Tuple* t) {
    maybe_dec_or_remove_counted_cptr(db, t->subject);
    maybe_dec_or_remove_counted_cptr(db, t->predicate);
    maybe_dec_or_remove_counted_cptr(db, t->object);

    t->is_deleted = 1;
}

Tuple** query(Database* db, TupleNoun subject, TupleNoun predicate, TupleNoun object) {

}
