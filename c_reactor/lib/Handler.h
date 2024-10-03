//
// Created by fox on 10/2/24.
//

#ifndef REACTOR_HANDLER_H
#define REACTOR_HANDLER_H

#include <functional>
//#include <boost/uuid/uuid.hpp>
//#include <boost/uuid/random_generator.hpp>
#include "Tuple.h"

namespace foxtalk {
    class Handler {
//    private:
//        boost::uuids::uuid id = boost::uuids::random_generator()();

    public:
//        boost::uuids::uuid get_uuid() {
//            return id;
//        }
//
        virtual Tuple* get_query() const = 0;
        virtual void handle_results(TupleVec tv, std::function<void(Tuple)> claim) const = 0;
    };
}

#endif //REACTOR_HANDLER_H
