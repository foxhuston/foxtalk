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
    /**
     * This is the runtime communication buffer, fixed to 10Mb by the constant above.
     * The first `sizeof(foxtalk_size_t)` bytes are the number of tuples in the buffer, which
     * are NOT GUARANTEED TO BE UNIQUE! After that is the standard serialization of Tuples,
     * which can be seen in `foxtalk_tuple.h`.
     */
    inline uint8_t _foxtalk_ipc_buffer[FOXTALK_IPC_BUFFER_SIZE];

    ///// USER MUST IMPLEMENT /////

    // from the <q, a, S, I, O> in the paper...
    void init(); // Q tuple is in the buffer after this
    void free_tuple();
    void register_initial_tuples(); // initial O in the buffer after this
    void handle();  // this is a
    void teardown();
    bool poll();
}

#endif // REACTOR_FOXTALK_HANDLER_H