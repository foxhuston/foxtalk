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
            ReactorSet<Tuple>::type result_tuples {};
            auto q = h->get_query();

            for (auto t: db.get_tuples()) {
//                std::cout << "Checking query " << *q << " vs tuple " << t << std::endl;

                if (!q->getSubject()->is_query()
                    && *t.getSubject() != *q->getSubject()) {
                    continue;
                }

                if (!q->getPredicate()->is_query()
                    && *t.getPredicate() != *q->getPredicate()) {
                    continue;
                }

                if (!q->getObject()->is_query()
                    && *t.getObject() != *q->getObject()) {
                    continue;
                }

                result_tuples.insert(t);
            }

            if(result_tuples.size() > 0) {
                if(!did_tuples_trigger_handler(result_tuples, h)) {
                    TupleVec result_tuples_vec{};

                    tuples_triggered_handler_insert(result_tuples, h);

                    for(auto t : result_tuples) {
                        result_tuples_vec.push_back(t);
                    }

                    // If not, then run the handler with these results.
                    h->handle_results(result_tuples_vec, [this, result_tuples](Tuple new_tuple) {
                        for (auto tr: result_tuples) {
                            tuple_prov_insert(tr, new_tuple);
                        }
                        this->claim(new_tuple);
                    });
                }
            }
        }
    }

    void Reactor::tuples_triggered_handler_insert(ReactorSet<Tuple>::type from, const Handler *to) {
        if(!tuples_triggered_handler.contains(from)) {
            tuples_triggered_handler.insert({ from, {} });
        }

        tuples_triggered_handler.at(from).insert(to);

    }

    bool Reactor::did_tuples_trigger_handler(ReactorSet<Tuple>::type from, const Handler *to) {
        return tuples_triggered_handler.contains(from)
               && tuples_triggered_handler.at(from).contains(to);
    }

} // foxtalk