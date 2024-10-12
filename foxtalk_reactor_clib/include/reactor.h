//
// Created by fox on 10/11/24.
//

#ifndef REACTOR_REACTOR_H
#define REACTOR_REACTOR_H

#include "vendor/kuzu.hpp"
#include "foxtalk_triple.h"

class Reactor {
private:
    std::shared_ptr<kuzu::main::Database> database;
    std::unique_ptr<kuzu::main::Connection> connection;

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
