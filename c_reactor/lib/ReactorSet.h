//
// Created by fox on 10/2/24.
//

#ifndef REACTOR_REACTORSET_H
#define REACTOR_REACTORSET_H

#include <unordered_set>
#include <unordered_map>
#include <boost/functional/hash.hpp>

#include "Tuple.h"

#include "gc_allocator.h"

namespace foxtalk {

    template<typename T>
    struct ReactorSet {
        typedef std::unordered_set<T, boost::hash<T>, std::equal_to<T>, traceable_allocator<T>> type;
    };

    template<>
    struct ReactorSet<const Tuple*> {
        template<typename T>
        struct deref_equal_to {
            bool operator()(const T x, const T y) const {
                return *x == *y;
            }
        };

        // I need to explain this. When I was using the gccpp library, it was taking over
        // `new` in all of C++, which I didn't really want, here. I wanted the GC to be relegated to
        // the database, and specifically just handle that. This wasn't a problem at first, until
        // I tried linking it with the Vulkan application, at which point the GC crashed the whole program.
        // So.
        // I'm using the regular C stuff, but now _everything_is_pointers_, and I need my hashsets to
        // actually use the underlying equality for tuples specifically. I made these generic because
        // I am compelled to put type parameters into everything I make, and now I need to be able to
        // swap out the dereferencing equality template thing specifically when I'm dealing with tuples.
        // Also:
        // Omg, template specialization to the rescue. I can't believe this worked!
        typedef std::unordered_set<const Tuple*, boost::hash<const Tuple*>, deref_equal_to<const Tuple*>, traceable_allocator<const Tuple*>> type;
    };

    template<typename T>
    struct ReactorVec {
        typedef std::vector<T, traceable_allocator<T>> type;
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
