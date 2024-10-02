//
// Created by fox on 10/1/24.
//

#include <queue>
#include <ranges>

#include "Reactor.h"

namespace views = std::ranges::views;

namespace foxtalk {
    void Reactor::claim(const Tuple &tuple) {
        db.add_tuple(tuple);
    }

    void Reactor::remove(const Tuple &tuple) {
        std::queue<Tuple> workQueue {};
        ReactorSet<Tuple>::type seen {};

        workQueue.push(tuple);

        while(!workQueue.empty()) {
            std::cout << "DEBUG REMOVE: WORK QUEUE IS " << workQueue.size() << std::endl;
            auto curr = workQueue.front();
            workQueue.pop();

            std::cout << "DEBUG REMOVE: HAVE WE SEEN " << curr << "?" << std::endl;

            // If we've already removed this, skip it.
            if(!seen.contains(curr)) {
                std::cout << "DEBUG REMOVE: REMOVING " << curr << std::endl;
                // Otherwise, remember that we've seen it.
                seen.insert(curr);

                // Enqueue all the things this tuple has created...
                if(tuple_provenance.contains(curr)) {
                    for (auto tup: tuple_provenance.at(curr)) {
                        std::cout << "DEBUG REMOVE: ENQUEUEING " << tup << std::endl;
                        workQueue.push(tup);
                    }
                }

                // And finally, remove it:
                db.remove_tuple(curr);
            }
        }
    }

    void Reactor::add_handler(const Handler *handler) {
        handlers.push_back(handler);
    }

    void Reactor::tuple_prov_insert(Tuple from, Tuple to) {
        if(!tuple_provenance.contains(from)) {
            tuple_provenance.insert({ from, {} });
        }

        tuple_provenance.at(from).insert(to);
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
                        tuple_prov_insert(tr, new_tuple);
                    }
                    this->claim(new_tuple);
                });
            }
        }
    }

} // foxtalk