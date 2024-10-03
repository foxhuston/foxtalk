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
        ReactorSet<const Tuple *>::type _all_tuples;

    public:
        Db() { }

        void add_tuple(const Tuple* tuple);

        // TODO: FOR TESTING ONLY!
        [[nodiscard]] const ReactorSet<const Tuple *>::type& get_tuples() const;

        // TODO: FOR TESTING ONLY!
        size_t size() const {
            return _all_tuples.size();
        }

        [[maybe_unused]] void remove_tuple(const Tuple* tuple);
    };

} // foxtalk

#endif //REACTOR_DB_H
