//
// Created by fox on 10/22/24.
//

#ifndef REACTOR_FOXTALK_HANDLER_HPP
#define REACTOR_FOXTALK_HANDLER_HPP

#include "foxtalk_handler.h"

class Handler {
protected:
    void claim(Tuple&& n) { claims.push_back(std::move(n)); }
    virtual void handle(const std::vector<Tuple>& queryResults) = 0;

public:
    std::vector<Tuple> claims {}; // should be private, but I need to test...

    virtual bool matches(const Tuple& n) = 0;

    void ffi_handle(uint8_t *buffer) {
        // Initialize
        claims.clear();

        // How many tuples in query result set?
        auto [query_results, read_bytes] = read_vec_from_buffer<Tuple>(buffer, 0);

        // Actually work with the results
        handle(query_results);

        // Serialize
        write_vec_to_buffer<Tuple>(buffer, 0, claims);
    }
};


#endif //REACTOR_FOXTALK_HANDLER_HPP
