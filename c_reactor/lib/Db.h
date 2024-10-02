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

#include "ReactorSet.h"

namespace foxtalk {

    class Db {
    private:
//        TupleVec _all_tuples { };
//        std::unordered_set<Tuple, std::hash<Tuple>, std::equal_to<Tuple>, traceable_allocator<Tuple>> _all_tuples { };
        ReactorSet<Tuple>::type _all_tuples;

    public:
        Db() { }

        void add_tuple(const Tuple& tuple);
        [[nodiscard]] const ReactorSet<Tuple>::type& get_tuples() const;

        // TODO: FOR TESTING ONLY!
        size_t size() const {
            return _all_tuples.size();
        }

        [[maybe_unused]] void remove_tuple(const Tuple& tuple);
    };

} // foxtalk

#endif //REACTOR_DB_H
