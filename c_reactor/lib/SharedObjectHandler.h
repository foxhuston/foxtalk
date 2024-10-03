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
#include "ReactorSet.h"
#include "Tuple.h"

namespace foxtalk {
    struct SharedObjectHandler : Handler {
    private:
        typedef Tuple* (*GetQuery)();
        typedef void (*HandleResults)(ReactorVec<const Tuple*>::type, std::function<void(const Tuple*)>);

        void *handle;
        GetQuery _get_query;
        HandleResults _handle_results;

    public:
        SharedObjectHandler(const char *file_name) {
            handle = dlopen(file_name, RTLD_LOCAL | RTLD_NOW);
            if(handle == nullptr) {
                throw std::runtime_error(std::format("Could not load dynamic handler for {0}! ({1})", file_name, dlerror()));
            }
            _get_query = reinterpret_cast<GetQuery>(dlsym(handle, "get_query"));
            if(_get_query == nullptr) {
                throw std::runtime_error(std::format("Could not find get_query for {0}! ({1})", file_name, dlerror()));
            }

            _handle_results = reinterpret_cast<HandleResults>(dlsym(handle, "handle_results"));
            if(_handle_results == nullptr) {
                throw std::runtime_error(std::format("Could not find handle_results for {0}! ({1})", file_name, dlerror()));
            }
        }

        Tuple *get_query() const override {
            return _get_query();
        }

        void handle_results(ReactorVec<const Tuple*>::type query_results, std::function<void(const Tuple*)> claim) const override {
            return _handle_results(query_results, claim);
        }

        ~SharedObjectHandler() {
            auto res = dlclose(handle);
            if(res != 0) {
                throw std::runtime_error(
                        std::format("Error when closing SharedObjectHandler: {0}", dlerror()));
            }
        }
    };

}

#endif //REACTOR_SHAREDOBJECTHANDLER_H
