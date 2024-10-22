    //
// Created by fox on 10/7/24.
//

#ifndef REACTOR_FOXTALK_HANDLER_H
#define REACTOR_FOXTALK_HANDLER_H

#include <cassert>
#include <cstring>
#include <string>
#include <unistd.h>
#include <cstdint>
#include <cmath>
#include <optional>

#include "foxtalk_tuple.h"

constexpr size_t FOXTALK_IPC_BUFFER_SIZE = 4096;

///// C FFI ////////////////////////////////////////////////////////////////////

extern "C" {
///// MY HANDLER ID /////
inline foxtalk_id_t my_handler_id;

///// RUNTIME COMMUNICATION BUFFER /////
inline uint8_t _foxtalk_ipc_triple_buffer[FOXTALK_IPC_BUFFER_SIZE];

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
#define FOXTALK_CLAIM(...) claim({__VA_ARGS__}, handlerEnvironment)
#define FOXTALK_GET_NEXT_QUERY_RESULT() getNextQueryResult(handlerEnvironment)
#define FOXTALK_REMOVE() remove(handlerEnvironment)
#define FOXTALK_REGISTER_HANDLE_QUERY(...) registerHandleQuery({__VA_ARGS__}, handlerEnvironment)


///// C++ API //////////////////////////////////////////////////////////////////

inline void write_to_ipc_buffer(const Tuple& t) {
    // size to the 0 position
    memset(_foxtalk_ipc_triple_buffer, 0, FOXTALK_IPC_BUFFER_SIZE);
    t.write_to_buffer(_foxtalk_ipc_triple_buffer, 0);
}

inline static Tuple read_from_ipc_buffer() {
    auto [t, bytes_read] = Tuple::read_from_buffer(_foxtalk_ipc_triple_buffer, 0);
    return std::move(t);
}

inline void claim(const Tuple& t, void* handlerEnvironment) {
    write_to_ipc_buffer(std::move(t));
    foxtalk_claim(handlerEnvironment);
}

inline void registerHandleQuery(const Tuple& t, void* handlerEnvironment) {
    write_to_ipc_buffer(std::move(t));
    foxtalk_registerHandleQuery(handlerEnvironment);
}

inline std::optional<Tuple> getNextQueryResult(void *handlerEnvironment) {
    if(foxtalk_getNextQueryResult(handlerEnvironment)) {
        auto [t, bytes_read] = Tuple::read_from_buffer(_foxtalk_ipc_triple_buffer, 0);
        return std::move(t);
    } else {
        return std::nullopt;
    }
}


#endif //REACTOR_FOXTALK_HANDLER_H
