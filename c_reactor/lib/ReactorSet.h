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

    template<typename K, typename V>
    void rm_set_insert(typename ReactorMap<K, typename ReactorSet<V>::type>::type& map, K k, V v) {
        if(!map.contains(k)) {
            map.insert({ k, {} });
        }

        map.at(k).insert(v);
    }

}


#endif //REACTOR_REACTORSET_H
