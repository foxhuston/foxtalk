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

    reactor->handlers[reactor->handler_count].called_with_results_count = 0;
    reactor->handlers[reactor->handler_count].called_with_results_alloc_count = initial_handler_size;
    reactor->handlers[reactor->handler_count].called_with_results = (TupleResult **)malloc(sizeof (TupleResult*) * initial_handler_size);
    memset(reactor->handlers[reactor->handler_count].called_with_results, 0, sizeof (TupleResult*) * initial_handler_size);

    reactor->handlers[reactor->handler_count].query_subject = subject;
    reactor->handlers[reactor->handler_count].query_predicate = predicate;
    reactor->handlers[reactor->handler_count].query_object = object;

    reactor->handlers[reactor->handler_count].handle_query_results = handlerFn;

    return reactor->handler_count++;
}

void reactor_removeHandler(Reactor *reactor, ReactorHandle reactor_handle) {
    assert(reactor_handle < reactor->handler_count);
    auto h = reactor->handlers[reactor_handle];
    assert(!h.is_deleted);

    for(size_t i = 0; i < h.called_with_results_count; i++) {
        free_db_query_results(h.called_with_results[i]);
    }

    reactor->handlers[reactor_handle].is_deleted = true;
}

void reactor_handler_tick(Reactor *reactor, ReactorHandler *current_handler) {
    size_t query_result_count = 0;
    auto query_results = db_query(reactor->db, current_handler->query_subject, current_handler->query_predicate, current_handler->query_object, &query_result_count);
    if(query_result_count <= 0) return;

    // Have we seen this result set before?
    for(size_t res_idx = 0; res_idx < current_handler->called_with_results_count; res_idx++) {
        if(tuple_results_eq(current_handler->called_with_results[res_idx], query_results)) {
            free_db_query_results(query_results);
            return; // Already called for this particular result set.
        }
    }

    // We haven't seen it. Record it...
    current_handler->called_with_results[current_handler->called_with_results_count] = query_results;
    current_handler->called_with_results_count++;
    assert(current_handler->called_with_results_count < current_handler->called_with_results_alloc_count);

    // Then actually run the handler
    current_handler->handle_query_results(reactor, query_results);
}

void reactor_tick(Reactor *reactor) {
    for(size_t i = 0; i < reactor->handler_count; i++) {
        auto current_handler = reactor->handlers + i;
        if(current_handler->is_deleted) { continue; }

        // Run the query
        reactor_handler_tick(reactor, current_handler);
    }
}
