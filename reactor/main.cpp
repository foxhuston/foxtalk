#include <iostream>

#include "foxtalk_handler.h"


/// EXAMPLE LIB ///

void register_query(size_t handler_id) {};


struct AwesomeStruct {

};

void init() {
    // Define query

    // <?, "is a", "husky">

    Triple query{
            TripleNoun{TripleNoun::NounType::U64, {.u64 = 69 }},
            TripleNoun{TripleNoun::NounType::U64, {.u64 = 420 }},
            TripleNoun{TripleNoun::NounType::CPtr, {.cptr = new AwesomeStruct{} }}
    };
    query.write_to_buffer();


    register_query(handler_id);
}

void free_tuple() {

}

void handle() {

}

void teardown() {

}

int main() {
    init();

//    for(size_t i = 0; i < buffer_size; i++) {
    for(size_t i = 0; i < *(size_t*)ipc_triple_buffer; i++) {
        std::cout << (uint32_t)ipc_triple_buffer[i] << " ";

        if(i % 10 == 9) {
            std::cout << std::endl;
        }
    }
}