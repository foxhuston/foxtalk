//
// Created by fox on 10/11/24.
//

#ifndef REACTOR_REACTOR_H
#define REACTOR_REACTOR_H

#include <vector>
#include "vendor/kuzu.hpp"
#include "foxtalk_triple.h"

struct Handler;
class Reactor;

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

    // Uniquely identifying a handler. Path, probably? But maybe this should be like a foxtalk_type_t
    std::string name;

    std::optional<std::string> cypher_query;
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

    std::unordered_map<std::string, std::unordered_set<Triple>> last_tick_created_triples;
    std::unordered_map<std::string, std::unordered_set<Triple>> current_tick_created_triples;

    static std::pair<std::vector<Handler>, std::vector<Handler>> split_handlers_by_initialization_state(const std::vector<Handler> &handlers);
    std::vector<Handler> getHandlers();

public:
    // No copy.
    Reactor(const Reactor&) = delete;
    void operator=(const Reactor&) = delete;

    Reactor(std::shared_ptr<kuzu::main::Database> db);
    void claim(const Triple& t, std::optional<std::string> creating_handler_name = std::nullopt) const;
    void remove(Triple t);

    void tick();
};


#endif //REACTOR_REACTOR_H
