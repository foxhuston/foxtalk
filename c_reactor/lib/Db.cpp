//
// Created by fox on 10/1/24.
//

#include <iostream>
#include "Db.h"

namespace foxtalk {
    void Db::add_tuple(const Tuple &tuple) {
//        std::cout << "Database Inserting tuple: " << tuple << std::endl;
//            _all_tuples.insert(tuple);
        _all_tuples.push_back(tuple);
    }

    void Db::remove_tuple(const Tuple &tuple) {
        auto _ = std::remove(_all_tuples.begin(), _all_tuples.end(), tuple);
//            _all_tuples.erase(tuple);
    }
    const TupleVec& Db::get_tuples() const {
        return _all_tuples;
    }
} // foxtalk