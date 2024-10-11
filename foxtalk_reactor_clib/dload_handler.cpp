//
// Created by fox on 10/11/24.
//

#include "foxtalk_handler.h"

#include <iostream>
#include <string>
#include <dlfcn.h>

using namespace std::literals;

void init() {

//    Triple setup {
//            TripleNoun { (uint64_t) my_handler_id }, // Probably want a foxtalk_id type.
//            TripleNoun { "is a"s },
//            TripleNoun { "aggregating handler"s },
//    };
//    setup.write_to_ipc_buffer();
//    _foxtalk_claim();

    // Subject is a path to an `so` file.
    Triple q {
        TripleNoun::query(),
        TripleNoun { "is a"s },
        TripleNoun { "so handler"s },
    };

    write_to_ipc_buffer(std::move(q));
}

void free_tuple() { }

void handle() {
    // /x/ is a handler
    auto res = getNextQueryResult();

    if(res.has_value()) {
        // dload magic...
        if(const auto& handler_path = res.value().get_subject<std::string>()) {

        }

        /*
         * <so path, "has foxtalk_id", foxtalk_id>
         * <foxtalk_id, "has dl handle", U64(dlopen handle)>
         * <foxtalk_id, "has init fn", CPtr(init ptr)>
         * <foxtalk_id, "has free_tuple fn", CPtr(free_tuple ptr)>
         * <foxtalk_id, "has handle fn", CPtr(handle ptr)>
         * <foxtalk_id, "has teardown fn", CPtr(teardown ptr)>
         */


    } else {
        std::cout << "WARNING: dload_handler `handle()` called with zero results..." << std::endl;
    }
}

void teardown() { }
