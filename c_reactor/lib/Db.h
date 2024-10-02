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
        TupleVec _all_tuples { };
//        std::unordered_set<Tuple, std::hash<Tuple>, std::equal_to<Tuple>, traceable_allocator<Tuple>> _all_tuples { };

    public:
        Db() { }

        void add_tuple(const Tuple& tuple);
        const std::vector<Tuple, traceable_allocator<Tuple>>& get_tuples() const;

        [[maybe_unused]] void remove_tuple(const Tuple& tuple);
    };

} // foxtalk

#endif //REACTOR_DB_H
