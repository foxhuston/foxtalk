//
// Created by fox on 10/1/24.
//

#ifndef REACTOR_DB_H
#define REACTOR_DB_H

#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "Tuple.h"
#include "ReactorSet.h"

namespace foxtalk {

    class Db {
    private:
        ReactorSet<const Tuple *>::type _all_tuples;

        // Memory Management Stuff
        std::unordered_map<const TupleNoun*, int> _tuplenoun_refcount {};

    public:
        Db() { }

        void inc_tuple_noun(const TupleNoun* tn);
        void dec_tuple_noun(const TupleNoun* tn);

        void add_tuple(const Tuple* tuple);
        void remove_tuple(const Tuple* tuple);

        void add_tuple_nouns(const Tuple* tuple);
        void remove_tuple_nouns(const Tuple* tuple);

        // TODO: FOR TESTING ONLY!
        [[nodiscard]] const ReactorSet<const Tuple *>::type& get_tuples() const;

        // TODO: FOR TESTING ONLY!
        size_t size() const {
            return _all_tuples.size();
        }

    };

} // foxtalk

#endif //REACTOR_DB_H
