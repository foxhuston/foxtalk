//
// Created by fox on 10/11/24.
//

#ifndef REACTOR_REACTOR_H
#define REACTOR_REACTOR_H

#include <vector>
#include "vendor/kuzu.hpp"
#include "foxtalk_triple.h"

struct Handler;
struct Reactor;

struct HandlerFunctionEnvironment {
    Triple *current_result;
    Handler *handler;
    Reactor *reactor;
};

struct Handler {
    typedef void (*Init)(HandlerFunctionEnvironment*);
    typedef void (*Handle)(HandlerFunctionEnvironment*);
    typedef void (*FreeTuple)();
    typedef void (*Teardown)();

    std::string cypher_query;
    bool isAggregating;
    uint8_t *handler_ipc_buffer;

    Init init;
    Handle handle;
    FreeTuple freeTuple;
    Teardown teardown;
};


class Reactor {
private:
    std::shared_ptr<kuzu::main::Database> database;
    std::unique_ptr<kuzu::main::Connection> connection;

    Triple query_single(const char* cypher);

    std::vector<Handler> getUninitializedHandlers(const std::vector<Handler>&);
    std::vector<Handler> getHandlers();

public:
    // No copy.
    Reactor(const Reactor&) = delete;
    void operator=(const Reactor&) = delete;

    Reactor(std::shared_ptr<kuzu::main::Database> db);

    void claim(Triple t);
    void remove(Triple t);

    void tick();
};


#endif //REACTOR_REACTOR_H
