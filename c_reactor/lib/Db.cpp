//
// Created by fox on 10/1/24.
//

#include "Db.h"

namespace foxtalk {
    void Db::add_tuple(const Tuple &tuple) {
//            _all_tuples.insert(tuple);
        _all_tuples.push_back(tuple);
    }

    void Db::remove_tuple(const Tuple &tuple) {
        auto _ = std::remove(_all_tuples.begin(), _all_tuples.end(), tuple);
//            _all_tuples.erase(tuple);
    }
    const std::vector<Tuple, traceable_allocator<Tuple>>& Db::get_tuples() const {
        return _all_tuples;
    }
} // foxtalk