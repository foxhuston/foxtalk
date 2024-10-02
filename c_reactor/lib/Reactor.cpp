//
// Created by fox on 10/1/24.
//

#include "Reactor.h"

namespace foxtalk {
    void Reactor::claim(const Tuple &tuple) {
        db.add_tuple(tuple);
    }

    void Reactor::remove(const Tuple &tuple) {
        // do this with work-queue
        db.remove_tuple(tuple);
    }

    void Reactor::add_handler(const Handler *handler) {
        handlers.push_back(handler);
    }

    void Reactor::tick() {
        for (auto h: handlers) {
            TupleVec result_tuples {};
            auto q = h->get_query();

            for (auto t: db.get_tuples()) {
                std::cout << "Checking query " << *q << " vs tuple " << t << std::endl;

                if (!q->getSubject()->is_query()
                    && *t.getSubject() != *q->getSubject()) {
                    std::cout << "DEBUG QUIT ON SUBJ" << std::endl;
                    continue;
                }

                if (!q->getPredicate()->is_query()
                    && *t.getPredicate() != *q->getPredicate()) {
                    std::cout << "DEBUG QUIT ON PRED" << std::endl;
                    continue;
                }

                if (!q->getObject()->is_query()
                    && *t.getObject() != *q->getObject()) {
                    std::cout << "DEBUG QUIT ON OBJ" << std::endl;
                    continue;
                }

                result_tuples.push_back(t);
            }

            if(result_tuples.size() > 0) {
                // Has this same set of tuples caused this activation before?


                // If not, then run the handler with these results.
                h->handle_results(result_tuples, [this, result_tuples](Tuple new_tuple) {
//                    std::cout << "REACTOR GOT WISHED TUPLE: " << new_tuple << std::endl;
                    for(auto tr : result_tuples) {

                        std::pair<Tuple, Tuple> x { tr, new_tuple };
                        tuple_provenance.insert(x);
                    }
                    this->claim(new_tuple);
                });
            }
        }
    }

} // foxtalk