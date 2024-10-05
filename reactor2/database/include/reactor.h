#ifndef __FOXTALK_REACTOR_H__
#define __FOXTALK_REACTOR_H__

#include "database.h"

#ifdef __cplusplus
extern "C" {
#endif
    typedef size_t ReactorHandle;
    typedef void (*ReactorHandlerFn)(TupleResult *query_results);

    typedef struct ReactorHandler_t {
        bool is_deleted;
        // You can't dynamically change your query; you've got to
        // unregister, make a new thing, and re-register. But I think
        // that for the way this is intended to be used, that's perfectly
        // valid, and will in fact reduce memory weirdness.
        TupleNoun query_subject;
        TupleNoun query_predicate;
        TupleNoun query_object;

        ReactorHandlerFn handle_query_results;
    } ReactorHandler;

    typedef struct Reactor_t {
        Database *db;

        size_t handler_count;
        size_t handler_alloc_count;
        ReactorHandler *handlers;
    } Reactor;


    Reactor* mkReactor();
    void freeReactor(Reactor *);

    Tuple *reactor_addTuple(Reactor *reactor, TupleNoun subject, TupleNoun predicate, TupleNoun object);
    void reactor_removeTuple(Reactor *reactor, Tuple *tuple);

    ReactorHandle reactor_addHandler(Reactor *reactor, TupleNoun subject, TupleNoun predicate, TupleNoun object, ReactorHandlerFn handlerFn);
    void reactor_removeHandler(Reactor *reactor, ReactorHandle reactor_handle);

    void tick(Reactor *reactor);

#ifdef __cplusplus
}
#endif

#endif // __FOXTALK_REACTOR_H__
