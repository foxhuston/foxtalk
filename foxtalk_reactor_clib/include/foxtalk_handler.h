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
#include "foxtalk_handler_api_fns.h"

constexpr size_t _foxtalk_ipc_buffer_size = 4096;

///// C FFI ////////////////////////////////////////////////////////////////////

extern "C" {
///// MY HANDLER ID /////
foxtalk_id_t my_handler_id;

///// RUNTIME COMMUNICATION BUFFER /////
uint8_t _foxtalk_ipc_triple_buffer[_foxtalk_ipc_buffer_size];

///// FROM THE RUNTIME /////
HandlerFunctions handler_fns;

///// USER MUST IMPLEMENT /////
void init();
void free_tuple();
void handle();
void teardown();
}

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

void claim(const Triple& t) {
    write_to_ipc_buffer(std::move(t));
    if(handler_fns.claim != nullptr) {
        handler_fns.claim();
    } else {
        throw std::runtime_error(
                std::format("Handler {} tried calling `claim` before it was set!", my_handler_id));
    }
}

std::optional<Triple> getNextQueryResult() {
    if(handler_fns.getNextQueryResult != nullptr) {
        if(handler_fns.getNextQueryResult()) {
            auto [t, bytes_read] = Triple::read_from_buffer(_foxtalk_ipc_triple_buffer, 0);
            return std::move(t);
        } else {
            return std::nullopt;
        }
    } else {
        throw std::runtime_error(
                std::format("Handler {} tried calling `claim` before it was set!", my_handler_id));
    }
}


#endif //REACTOR_FOXTALK_HANDLER_H
