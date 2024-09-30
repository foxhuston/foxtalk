#ifndef __REACTOR_H__
#define __REACTOR_H__

#include <cstdint>
#include <vector>

typedef void* TupleNoun;
typedef void* Tuple;
typedef void (*free_fn)(void *);

///// C++ Handlers must define these /////
std::vector<Tuple> *WhenHandler(Tuple queryResult);
Tuple GetQuery();

///// FFI /////
extern "C" {
    TupleNoun mk_tuple_noun_query();
    TupleNoun mk_tuple_noun_cptr_with_free(void* data, free_fn free_fn);
    TupleNoun mk_tuple_noun_symbol(const char *s);
    TupleNoun mk_tuple_noun_u64(uint64_t);
    TupleNoun mk_tuple_noun_i64(int64_t);

    Tuple mk_tuple(TupleNoun subject, TupleNoun predicate, TupleNoun object);

    TupleNoun get_tuple_subject(Tuple t);
    TupleNoun get_tuple_predicate(Tuple t);
    TupleNoun get_tuple_object(Tuple t);

    const char *get_tuple_noun_as_symbol(TupleNoun tn);
    const void* get_tuple_noun_as_cptr(TupleNoun tn);
    const uint64_t get_tuple_noun_as_u64(TupleNoun tn);
    const int64_t get_tuple_noun_as_i64(TupleNoun tn);

    ///// C++ API WRAPPERS /////
    Tuple get_query() {
        return GetQuery();
    }

    // TODO: Leaky.
    Tuple* when_handler(Tuple result, size_t *outCount) {
        auto r = WhenHandler(result);
        *outCount = r->size();

        Tuple* out = (Tuple *)malloc(sizeof(Tuple) * r->size());
        for(int i = 0; i < r->size(); i++) {
            out[i] = r->at(i);
        }

        return out;
    }
}

#endif // __REACTOR_H__
