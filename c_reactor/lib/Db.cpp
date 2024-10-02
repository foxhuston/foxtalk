//
// Created by fox on 10/1/24.
//

#include <iostream>
#include "Db.h"

namespace foxtalk {
    void Db::add_tuple(const Tuple &tuple) {
//        std::cout << "Database Inserting tuple: " << tuple << std::endl;
        _all_tuples.insert(tuple);
    }

    void Db::remove_tuple(const Tuple &tuple) {
        _all_tuples.erase(tuple);
    }
    const ReactorCache<Tuple>::Set& Db::get_tuples() const {
        return _all_tuples;
    }
} // foxtalk