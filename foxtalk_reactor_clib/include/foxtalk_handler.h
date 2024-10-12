//
// Created by fox on 10/7/24.
//

#ifndef REACTOR_FOXTALK_HANDLER_H
#define REACTOR_FOXTALK_HANDLER_H

#include <iostream>
#include <format>
#include <cassert>
#include <variant>
#include <cstring>
#include <string>
#include <unistd.h>
#include <cstdint>
#include <cmath>
#include <optional>

#include "foxtalk_triple.h"

constexpr size_t _foxtalk_ipc_buffer_size = 4096;

///// C FFI ////////////////////////////////////////////////////////////////////

extern "C" {
///// MY HANDLER ID /////
foxtalk_id_t my_handler_id;

///// RUNTIME COMMUNICATION BUFFER /////
uint8_t _foxtalk_ipc_triple_buffer[_foxtalk_ipc_buffer_size];

///// FROM THE RUNTIME /////
void foxtalk_claim(void *handlerEnvironment);
void foxtalk_registerHandleQuery(void *handlerEnvironment);
void foxtalk_remove(void *handlerEnvironment);
bool foxtalk_getNextQueryResult(void *handlerEnvironment);

///// USER MUST IMPLEMENT /////
void init(void *handlerEnvironment);
void free_tuple();
void handle(void* handlerEnvironment);
void teardown();
}

#define FOXTALK_INIT void init(void* handlerEnvironment)
#define FOXTALK_FREE_TUPLE void free_tuple()
#define FOXTALK_HANDLE void handle(void* handlerEnvironment)
#define FOXTALK_TEARDOWN void teardown()
#define FOXTALK_CLAIM(...) claim(__VA_ARGS__, handlerEnvironment)
#define FOXTALK_GET_NEXT_QUERY_RESULT() getNextQueryResult(handlerEnvironment)
#define FOXTALK_REMOVE() remove(handlerEnvironment)
#define FOXTALK_REGISTER_HANDLE_QUERY(...) registerHandleQuery(__VA_ARGS__, handlerEnvironment)


///// C++ API //////////////////////////////////////////////////////////////////

void write_to_ipc_buffer(const Triple& t) {
    // size to the 0 position
    memset(_foxtalk_ipc_triple_buffer, 0, _foxtalk_ipc_buffer_size);
    t.write_to_buffer(_foxtalk_ipc_triple_buffer, 0);
}

static Triple read_from_ipc_buffer() {
    auto [t, bytes_read] = Triple::read_from_buffer(_foxtalk_ipc_triple_buffer, 0);
    return std::move(t);
}

void claim(const Triple& t, void* handlerEnvironment) {
    write_to_ipc_buffer(std::move(t));
    foxtalk_claim(handlerEnvironment);
}

void registerHandleQuery(const Triple& t, void* handlerEnvironment) {
    write_to_ipc_buffer(std::move(t));
    foxtalk_registerHandleQuery(handlerEnvironment);
}

std::optional<Triple> getNextQueryResult(void *handlerEnvironment) {
    if(foxtalk_getNextQueryResult(handlerEnvironment)) {
        auto [t, bytes_read] = Triple::read_from_buffer(_foxtalk_ipc_triple_buffer, 0);
        return std::move(t);
    } else {
        return std::nullopt;
    }
}


#endif //REACTOR_FOXTALK_HANDLER_H
