//
// Created by fox on 10/2/24.
//

#ifndef REACTOR_REACTORSET_H
#define REACTOR_REACTORSET_H

#include <unordered_set>
#include <unordered_map>
#include <boost/functional/hash.hpp>
#include <gc_allocator.h>

namespace foxtalk {

    template<typename T>
    struct ReactorSet {
        typedef std::unordered_set<T, boost::hash<T>, std::equal_to<T>, traceable_allocator<T>> type;
    };

    template<typename T, typename V>
    struct ReactorMap {
        typedef std::unordered_map<T, V, boost::hash<T>, std::equal_to<T>, traceable_allocator<std::pair<const T, V>>> type;
    };

}


#endif //REACTOR_REACTORSET_H
