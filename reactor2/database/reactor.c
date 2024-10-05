#include <assert.h>
#include <string.h>
#include "database.h"
#include "reactor.h"

constexpr size_t initial_handler_size = 1000000;

Reactor *mkReactor() {
    Reactor* reactor = malloc(sizeof(Reactor));

    reactor->db = mkNewDatabase();

    reactor->handler_alloc_count = initial_handler_size;
    reactor->handler_count = 0;
    reactor->handlers = malloc(sizeof(ReactorHandler) * initial_handler_size);

    memset(reactor->handlers, 0, sizeof(ReactorHandler) * initial_handler_size);

    return reactor;
}

void freeReactor(Reactor *r) {
    freeDatabase(r->db);
    free(r->handlers);
    free(r);
}

Tuple *reactor_addTuple(Reactor *reactor, TupleNoun subject, TupleNoun predicate, TupleNoun object) {
    return db_addTuple(reactor->db, subject, predicate, object);
}

void reactor_removeTuple(Reactor *reactor, TupleNoun subject, TupleNoun predicate, TupleNoun object) {
    db_removeTuple(reactor->db, subject, predicate, object);
}

ReactorHandle reactor_addHandler(Reactor *reactor, TupleNoun subject, TupleNoun predicate, TupleNoun object, ReactorHandlerFn handlerFn) {
    assert(reactor->handler_count < reactor->handler_alloc_count);

    reactor->handlers[reactor->handler_count].is_deleted = false;

    reactor->handlers[reactor->handler_count].query_subject = subject;
    reactor->handlers[reactor->handler_count].query_predicate = predicate;
    reactor->handlers[reactor->handler_count].query_object = object;

    reactor->handlers[reactor->handler_count].handle_query_results = handlerFn;

    return reactor->handler_count++;
}

void reactor_removeHandler(Reactor *reactor, ReactorHandle reactor_handle) {
    assert(reactor_handle < reactor->handler_count);
    reactor->handlers[reactor_handle].is_deleted = true;
}

void tick(Reactor *reactor) {

}
