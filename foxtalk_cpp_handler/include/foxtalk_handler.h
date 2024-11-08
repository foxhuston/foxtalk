//
// Created by fox on 10/7/24.
//

#ifndef REACTOR_FOXTALK_HANDLER_H
#define REACTOR_FOXTALK_HANDLER_H

#include <vector>
#include <unistd.h>
#include <cstdint>

#include "foxtalk_tuple.h"

constexpr size_t FOXTALK_IPC_BUFFER_SIZE = 10 * 1024 * 1024; // 10Mb

///// C FFI ////////////////////////////////////////////////////////////////////

extern "C"
{
    ///// USER MUST IMPLEMENT /////

    // from the <q, a, S, I, O> in the paper...
    void init(uint8_t *buffer); // Q tuple is in the buffer after this
    void free_tuple(uint8_t *buffer);
    void register_initial_tuples(uint8_t *buffer); // initial O in the buffer after this
    void handle(uint8_t *buffer);  // this is a
    void teardown();
    bool poll();
}

#endif // REACTOR_FOXTALK_HANDLER_H