//
// Created by fox on 10/1/24.
//

#include <iostream>
#include <queue>

#include "gc.h"
#include "Reactor.h"

namespace views = std::ranges::views;

namespace foxtalk {
    void Reactor::claim(const Tuple *tuple) {
        db.add_tuple(tuple);
    }

    void Reactor::claim(const TupleNoun *subject, const TupleNoun *predicate, const TupleNoun *object) {
        claim(Tuple::mk(subject, predicate, object));
    }

    void Reactor::remove(const Tuple *tuple) {
        std::queue<const Tuple *> workQueue{};
        workQueue.push(tuple);
        remove(workQueue);
    }

    void Reactor::remove(ReactorSet<const Tuple *>::type tuples) {
        std::queue<const Tuple *> workQueue{};
        for (auto &t: tuples) {
            workQueue.push(t);
        }
        remove(workQueue);
    }

    void Reactor::remove(std::queue<const Tuple *> workQueue) {
        ReactorSet<const Tuple *>::type seen{};

        while (!workQueue.empty()) {
            std::cout << "DEBUG REMOVE: WORK QUEUE IS " << workQueue.size() << std::endl;
            auto curr = workQueue.front();
            workQueue.pop();

            std::cout << "DEBUG REMOVE: HAVE WE SEEN " << *curr << "?" << std::endl;

            // If we've already removed this, skip it.
            if (!seen.contains(curr)) {
                std::cout << "DEBUG REMOVE: NOPE! REMOVING " << *curr << std::endl;
                // Otherwise, remember that we've seen it.
                seen.insert(curr);

                // Enqueue all the things this tuple has created...
                if (tuple_provenance.contains(curr)) {
                    for (auto &tup: tuple_provenance.at(curr)) {
                        std::cout << "DEBUG REMOVE: ENQUEUEING " << *tup << std::endl;
                        workQueue.push(tup);
                    }
                }

                // If this was part of a set that triggered a handler, we'll need to run the handler again,
                // and remove anything that handler had caused to exist.
                std::vector<ReactorSet<const Tuple *>::type> keys_to_remove{};
                for (auto &kv: tuples_triggered_handler) {
                    if (kv.first.contains(curr)) {
                        std::cout << "DEBUG REMOVE: HANDLER_PROVENANCE " << *curr << " / " << &(kv.first) << std::endl;
                        keys_to_remove.push_back(kv.first);

                        for(auto triggered_handler : kv.second) {
                            if(tuple_handler_provenance.contains(triggered_handler)) {
                                for (auto generated_tuple: tuple_handler_provenance.at(triggered_handler)) {
                                    workQueue.push(generated_tuple);
                                }
                            } else {
                                std::cout << "WARNING! Trying to remove tuples from removed handler " << triggered_handler << std::endl;
                            }
                        }
                    }
                }

                std::cout << "DEBUG REMOVE: tuple_triggered_handler erase: " << *curr << ")" << std::endl;
                for (auto &k: keys_to_remove) {
                    tuples_triggered_handler.erase(k);
                }

                // And finally, remove it:
                std::cout << "DEBUG REMOVE: Calling db.remove_tuple(" << *curr << ")" << std::endl;
                db.remove_tuple(curr);
                assert(!db.get_tuples().contains(curr));
            }
        }
    }

    void Reactor::add_handler(const Handler *handler) {
        std::lock_guard<std::mutex> guard(handlerMutex);
        std::cout << "Adding handler " << handler << std::endl;
        handlers.insert(handler);
    }

    void Reactor::remove_handler(const Handler *handler) {
        std::lock_guard<std::mutex> guard(handlerMutex);
        std::cout << "Removing handler " << handler << std::endl;

        std::queue<const Tuple *> workQueue {};
        for(auto generated_tuple : tuple_handler_provenance.at(handler)) {
            workQueue.push(generated_tuple);
        }
        remove(workQueue);

        handlers.erase(handler);
        if (tuple_handler_provenance.contains(handler)) {
            remove(tuple_handler_provenance.at(handler));
            tuple_handler_provenance.erase(handler);
        }

        for (auto &kv: tuples_triggered_handler) {
            kv.second.erase(handler);
        }
    }

    ReactorSet<const Tuple *>::type Reactor::query(const Tuple *q) {
        ReactorSet<const Tuple *>::type result_tuples{};

        for (auto t: db.get_tuples()) {
            if (!q->getSubject()->is_query()
                && *t->getSubject() != *q->getSubject()) {
                continue;
            }

            if (!q->getPredicate()->is_query()
                && *t->getPredicate() != *q->getPredicate()) {
                continue;
            }

            if (!q->getObject()->is_query()
                && *t->getObject() != *q->getObject()) {
                continue;
            }

            result_tuples.insert(t);
        }

        return result_tuples;
    }

    void Reactor::tick() {
        std::lock_guard<std::mutex> guard(handlerMutex);
        for (auto h: handlers) {
            auto result_tuples = query(h->get_query());

            if (result_tuples.size() > 0) {
                if (!did_tuples_trigger_handler(result_tuples, h)) {
                    ReactorVec<const Tuple *>::type result_tuples_vec{};

                    tuples_triggered_handler_insert(result_tuples, h);

                    for (auto &t: result_tuples) {
                        result_tuples_vec.push_back(t);
                    }

                    // If not, then run the handler with these results.
                    h->handle_results(result_tuples_vec, [this, h, result_tuples](const Tuple *new_tuple) {
                        for (auto tr: result_tuples) {
                            rm_set_insert(tuple_handler_provenance, h, new_tuple);
                            rm_set_insert(tuple_provenance, tr, new_tuple);
                        }
                        this->claim(new_tuple);
                    });
                }
            }
        }
    }

    void Reactor::tuples_triggered_handler_insert(ReactorSet<const Tuple *>::type from, const Handler *to) {
        if (!tuples_triggered_handler.contains(from)) {
            tuples_triggered_handler.insert({from, {}});
        }

        tuples_triggered_handler.at(from).insert(to);
    }

    bool Reactor::did_tuples_trigger_handler(ReactorSet<const Tuple *>::type from, const Handler *to) {
        return tuples_triggered_handler.contains(from)
               && tuples_triggered_handler.at(from).contains(to);
    }

} // foxtalk