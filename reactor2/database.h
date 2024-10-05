#ifndef __FOXTALK_DATABASE_H__
#define __FOXTALK_DATABASE_H__

#include <stdint.h>
#include "symbol.h"

typedef void (*FreeFn)(void *);
typedef size_t TupleRef;

typedef struct CPtrWithFree_t {
    void *data;
    FreeFn free_fn;
} CPtrWithFree;

typedef struct RefCountedCPtrWithFree_t {
    size_t ref_count;
    void *data;
    FreeFn free_fn;
} RefCountedCPtrWithFree;

typedef struct TupleNoun_t {
    enum Type {
        Query,
        Pair,
        Sym,
        CPtr,
        U64,
        I64
    } type;

    union Data {
//        TuplePair pair;
        CPtrWithFree cptr;

        // Never need cleanup.
        Symbol *symbol;
        uint64_t u64;
        int64_t i64;
    } data;
} TupleNoun;

typedef struct Tuple_t {
    bool is_deleted;
    TupleNoun subject;
    TupleNoun predicate;
    TupleNoun object;
} Tuple;

typedef struct Database_t {
    size_t cptr_count;
    size_t cptr_alloc_size;
    RefCountedCPtrWithFree *cptrs;

    size_t tuple_count;
    size_t alloc_size;
    Tuple *tuples;
} Database;


Database* mkNewDatabase();
void freeDatabase(Database *);

Tuple* addTuple(Database*, TupleNoun subject, TupleNoun predicate, TupleNoun object);
void removeTuple(Database*, Tuple*);

//Tuple** query(Database*, TupleNoun subject, TupleNoun predicate, TupleNoun object);

#endif // __FOXTALK_DATABASE_H__
