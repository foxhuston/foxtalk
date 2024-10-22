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

    void ffi_handle() {
        // Initialize
        claims.clear();

        // How many tuples in query result set?
        auto [tuple_count, current_position] = read_t_from_buffer<foxtalk_size_t>(_foxtalk_ipc_buffer, 0);

        // Deserialize the results
        std::vector<Tuple> query_results {};
        for(int i = 0; i < tuple_count; i++) {
            auto [t, read_bytes] = Tuple::read_from_buffer(_foxtalk_ipc_buffer, current_position);
            current_position += read_bytes;
            query_results.push_back(t);
        }

        // Actually work with the results
        handle(query_results);

        // Serialize

        auto count_bytes = write_t_to_buffer(_foxtalk_ipc_buffer, 0, (foxtalk_size_t) claims.size());
        current_position = count_bytes;

        for(auto c : claims) {
            current_position += c.write_to_buffer(_foxtalk_ipc_buffer, current_position);
        }


    }
};


#endif //REACTOR_FOXTALK_HANDLER_HPP
