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

    write_to_ipc_buffer(q);
}

void free_tuple() { }

void handle() {
    // /x/ is a handler
    auto res = getNextQueryResult();

    if(res.has_value()) {
        if(auto handler_path = res.value().get_subject<std::string>()) {
            auto handle = dlopen(handler_path->c_str(), RTLD_LOCAL | RTLD_NOW);

            auto init_fn = dlsym(handle, "init");
            auto free_tuple_fn = dlsym(handle, "free_tuple");
            auto handle_fn = dlsym(handle, "handle");
            auto teardown_fn = dlsym(handle, "teardown");

            claim({ {*handler_path}, {"has dl handle"}, {handle} });
            claim({ {handle}, {"has init fn"}, {init_fn} });
            claim({ {handle}, {"has free_tuple fn"}, {free_tuple_fn} });
            claim({ {handle}, {"has handle fn"}, {handle_fn} });
            claim({ {handle}, {"has teardown fn"}, {teardown_fn} });
        }
    } else {
        std::cout << "WARNING: dload_handler `handle()` called with zero results..." << std::endl;
    }
}

void teardown() {
    auto t = read_from_ipc_buffer();
    if(auto pred = t.get_predicate<std::string>()) {
        if(pred == "has dl handle") {
            if(auto obj = t.get_object<void*>()) {
                dlclose(*obj);
            } else {
                std::cerr << "ERROR: Tried to clean up the tuple " << t << ", but its object is not a `void*`!" << std::endl;
            }
        }
    }
}
