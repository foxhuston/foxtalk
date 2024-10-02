//
// Created by fox on 10/2/24.
//

#ifndef REACTOR_REACTORCACHE_H
#define REACTOR_REACTORCACHE_H

#include <unordered_set>
#include <boost/functional/hash.hpp>
#include <gc_allocator.h>

namespace foxtalk {

    template<typename T>
    struct ReactorCache {
        typedef std::unordered_set<T, boost::hash<T>, std::equal_to<T>, traceable_allocator<T>> Set;
    };

}


#endif //REACTOR_REACTORCACHE_H
