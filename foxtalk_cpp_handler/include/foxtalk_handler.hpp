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

    bool ffi_matches(uint8_t *buffer) {
        auto [t, read_bytes] = Tuple::read_from_buffer(buffer, 0);
        return matches(t);
    }

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


// This is a GTEST-style API, where you first write FOXTALK_HANDLER(Name, qr) { ... }, and then FOXTALK_HANDLER_QUERY(NAME, q) { return ... }
// It's pretty cool, but a bit unwieldy given that you can choose to do init/teardowns or not. Although perhaps I'm
// not quite thinking about this correctly...?
//
//#define FOXTALK_HANDLER_BOD(T, qr) void T::handle(const std::vector<Tuple>& qr)
//#define FOXTALK_HANDLER_DEF(T, qr) class T : public Handler { virtual bool matches(const Tuple&); virtual void handle(const std::vector<Tuple>&); }
//#define FOXTALK_HANDLER(T, qr) FOXTALK_HANDLER_DEF(T, qr); FOXTALK_HANDLER_BOD(T, qr)
//#define FOXTALK_HANDLER_MATCHES(T, inp) bool T::matches(const Tuple& inp)

#define FOXTALK_FFI_HANDLER_REG(T) static T* T ## _instance = nullptr; \
    void init() { T ## _instance = new T(); } \
    void free_tuple() { throw new std::runtime_error("Unimplemented!"); } \
    bool matches() { return T ## _instance ->ffi_matches(_foxtalk_ipc_buffer); } \
    void handle() { T ## _instance ->ffi_handle(_foxtalk_ipc_buffer); } \
    void teardown() { delete T ## _instance; } \

//#define FOXTALK_FFI_HANDLER(T, qr) \
//    FOXTALK_HANDLER_DEF(T, qr);    \
//    FOXTALK_FFI_HANDLER_REG(T)     \
//    FOXTALK_HANDLER_BOD(T, qr)


#endif //REACTOR_FOXTALK_HANDLER_HPP
