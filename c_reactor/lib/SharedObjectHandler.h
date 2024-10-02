//
// Created by fox on 10/2/24.
//

#ifndef REACTOR_SHAREDOBJECTHANDLER_H
#define REACTOR_SHAREDOBJECTHANDLER_H

#include <optional>
#include <cstring>
#include <errno.h>
#include <dlfcn.h>

#include "Handler.h"

namespace foxtalk {
    struct SharedObjectHandler : Handler {
    private:
        typedef Tuple* (*GetQuery)();
        typedef void (*HandleResults)(TupleVec, std::function<void(Tuple)>);

        void *handle;
        GetQuery _get_query;
        HandleResults _handle_results;

    public:
        SharedObjectHandler(const char *file_name) {
            handle = dlopen(file_name, RTLD_LOCAL | RTLD_NOW);
            if(handle == nullptr) {
                throw std::runtime_error(std::format("Could not load dynamic handler! ({0})", strerror(errno)));
            }
            _get_query = reinterpret_cast<GetQuery>(dlsym(handle, "get_query"));
            if(_get_query == nullptr) {
                throw std::runtime_error(std::format("Could not find get_query! ({0})", strerror(errno)));
            }

            _handle_results = reinterpret_cast<HandleResults>(dlsym(handle, "handle_results"));
            if(_handle_results == nullptr) {
                throw std::runtime_error(std::format("Could not find handle_results! ({0})", strerror(errno)));
            }
        }

        Tuple *get_query() const override {
            return _get_query();
        }

        void handle_results(TupleVec query_results, std::function<void(Tuple)> claim) const override {
            return _handle_results(query_results, claim);
        }

        ~SharedObjectHandler() {
            dlclose(handle);
        }
    };

}

#endif //REACTOR_SHAREDOBJECTHANDLER_H
