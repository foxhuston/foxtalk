//
// Created by fox on 10/1/24.
//

#ifndef REACTOR_DB_H
#define REACTOR_DB_H

#include <vector>
#include <unordered_set>

#include "gc_cpp.h"
#include "gc_allocator.h"
#include "Tuple.h"

namespace foxtalk {

    class Db {
    private:
        traceable_allocator<Tuple> gcAllocator {};
        std::vector<Tuple, traceable_allocator<Tuple>> _all_tuples { };
//        std::unordered_set<Tuple, std::hash<Tuple>, std::equal_to<Tuple>, traceable_allocator<Tuple>> _all_tuples { };

    public:
        Db() { }

        void add_tuple(const Tuple& tuple) {
//            _all_tuples.insert(tuple);
            _all_tuples.push_back(tuple);
        }

        void remove_tuple(const Tuple& tuple) {
            auto _ = std::remove(_all_tuples.begin(), _all_tuples.end(), tuple);
//            _all_tuples.erase(tuple);
        }
    };

} // foxtalk

#endif //REACTOR_DB_H
