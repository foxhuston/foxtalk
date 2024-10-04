//
// Created by fox on 10/1/24.
//

#include <iostream>
#include <queue>

// #include "gc.h"
#include "Reactor.h"

#define REACTOR_TRACE
#define REACTOR_MEM_DEBUG

namespace views = std::ranges::views;

namespace foxtalk {
    void Reactor::claim(const Tuple *tuple) {
        std::lock_guard<std::mutex> guard(tupleMutex);
        //        GC_init();
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
        for (auto t: tuples) { workQueue.push(t); }
        remove(workQueue);
    }

    void Reactor::remove(std::queue<const Tuple *> workQueue) {
#ifdef REACTOR_TRACE
        std::cout << "DEBUG Called Remove on workQueue with " << workQueue.size() << " item(s)." << std::endl;
#endif

        std::lock_guard<std::mutex> guard(handlerMutex);
        std::unordered_set<const Tuple *> seen_ptrs{};

        while (!workQueue.empty()) {
            //      std::cout << "DEBUG REMOVE: WORK QUEUE IS " << workQueue.size() <<
            //      std::endl;
            auto tuple_to_remove = workQueue.front();
            workQueue.pop();

#ifdef REACTOR_TRACE
            std::cout << "  DEBUG: WILL REMOVE " << *tuple_to_remove << " @ " << tuple_to_remove << std::endl;
#endif

            // If we've already removed this in this pass, or if it's in the current
            // removal list, skip it.
            if (!seen_ptrs.contains(tuple_to_remove) && !tuples_to_remove.contains(tuple_to_remove)) {
                //        std::cout << "DEBUG REMOVE: UNSEEN TUPLE! REMOVING " <<
                //        *tuple_to_remove << std::endl;
                // Otherwise, remember that we've seen it.
                seen_ptrs.insert(tuple_to_remove);

                // Enqueue all the things this tuple has created...
                if (tuple_provenance.contains(tuple_to_remove)) {
                    for (auto tup: tuple_provenance.at(tuple_to_remove)) {
                        //            std::cout << "DEBUG REMOVE: TUPLE_PROV ENQUEUEING " <<
                        //            *tup << std::endl;
                        workQueue.push(tup);
                    }
                }

                // remove this provenance key, AND...
                tuple_provenance.erase(tuple_to_remove);

                // remove references to this tuple in any provenance value sets:
                for (auto &[key, value_tups]: tuple_provenance) { value_tups.erase(tuple_to_remove); }

                // If this was part of a set that triggered a handler, we'll need to run
                // the handler again, and remove anything that handler had caused to
                // exist.
#ifdef REACTOR_TRACE
                std::cout << "Checking if " << *tuple_to_remove << " @ " << tuple_to_remove << " triggered a handler..."
                          << std::endl;
#endif
                ReactorSet<const Handler *>::type handler_tuples_to_remove{};
                if (tuples_in_triggerset_hash.contains(tuple_to_remove)) {
                    for (size_t triggerset_hash: tuples_in_triggerset_hash.at(tuple_to_remove)) {
#ifdef REACTOR_TRACE
                        std::cout << "  It did! Processing triggerset_hash " << triggerset_hash << std::endl;
#endif
                        for (auto handler: triggered_handlers_by_triggerset_hash.at(triggerset_hash)) {
                            if (tuple_handler_provenance.contains(handler)) {
                                for (auto handler_generated_tuple: tuple_handler_provenance.at(handler)) {
#ifdef REACTOR_TRACE
                                    std::cout << "  Enqueueing handler-generated tuple " << *handler_generated_tuple
                                              << " @ " << handler_generated_tuple << std::endl;
#endif
                                    workQueue.push(handler_generated_tuple);
                                }
                            }
                        }
                        triggered_handlers_by_triggerset_hash.erase(triggerset_hash);
                    }

#ifdef REACTOR_TRACE
                    std::cout << "  Erasing tuple from tuples_in_triggerset_hash " << *tuple_to_remove << " @ "
                              << tuple_to_remove << std::endl;
#endif

                    tuples_in_triggerset_hash.erase(tuple_to_remove);
                }


                // Wishes end up in tuple_handler_provenance, so we need to also just manually check that.
                for (auto &[h, generated_tuples]: tuple_handler_provenance) { generated_tuples.erase(tuple_to_remove); }

#ifdef REACTOR_TRACE
                std::cout << "INSERTING FOR REMOVAL " << *tuple_to_remove << " @ " << tuple_to_remove
                          << " into tuples_to_remove" << std::endl;
#endif
                tuples_to_remove.insert(tuple_to_remove);
            }
        }
    }

    void Reactor::add_handler(const Handler *handler) {
        std::lock_guard<std::mutex> guard(handlerMutex);
        //        std::cout << "Adding handler " << handler << std::endl;
        handlers.insert(handler);
    }

    void Reactor::remove_handler(const Handler *handler_to_remove) {
        std::lock_guard<std::mutex> guard(tupleMutex);
        //        std::cout << "Removing handler_to_remove " << handler_to_remove << std::endl;

        std::queue<const Tuple *> workQueue{};
        for (auto generated_tuple: tuple_handler_provenance.at(handler_to_remove)) { workQueue.push(generated_tuple); }

        for (auto &[_hash, triggered_handlers]: triggered_handlers_by_triggerset_hash) {
            triggered_handlers.erase(handler_to_remove);
        }

        if (tuple_handler_provenance.contains(handler_to_remove)) {
            for (auto provenance_tuple: tuple_handler_provenance.at(handler_to_remove)) {
                workQueue.push(provenance_tuple);
            }
            tuple_handler_provenance.erase(handler_to_remove);
        }

        assert(handlers.erase(handler_to_remove) == 1);
        remove(workQueue);
    }

    void Reactor::tick() {
        std::lock_guard<std::mutex> guardH(handlerMutex);

        for (auto h: handlers) {
            auto handler_query = h->get_query();
            auto result_tuples =
                    query(handler_query->getSubject(), handler_query->getPredicate(), handler_query->getObject());
            delete handler_query;// TODO: Yikes.

            if (!result_tuples.empty()) {
                if (!did_tuples_trigger_handler(result_tuples, h)) {
                    ReactorVec<const Tuple *>::type result_tuples_vec{};

                    tuples_triggered_handler_insert(result_tuples, h);

                    for (auto &t: result_tuples) { result_tuples_vec.push_back(t); }

                    // If not, then run the handler with these results.
                    h->handle_results(result_tuples_vec, [this, h, result_tuples](const Tuple *new_tuple) {
#ifdef REACTOR_TRACE
                        std::cout << "Wish <" << new_tuple->getSubject() << ", " << new_tuple->getPredicate() << ", "
                                  << new_tuple->getObject() << ">" << std::endl;
#endif

                        for (auto tr: result_tuples) {
                            rm_set_insert(tuple_handler_provenance, h, new_tuple);
                            rm_set_insert(tuple_provenance, tr, new_tuple);
                        }
                        this->claim(new_tuple);
                    });
#ifdef REACTOR_TRACE
                } else {
                    std::cout << "Handler already triggered!" << std::endl;
#endif
                }
            }
        }

        // Clean up removed tuples.
        std::lock_guard<std::mutex> guardT(tupleMutex);
        for (auto tuple_to_delete: tuples_to_remove) {
#ifdef REACTOR_TRACE
            std::cout << "FOUND IN TUPLES_TO_REMOVE: " << *tuple_to_delete << " @ " << tuple_to_delete << std::endl;
#endif

#ifdef REACTOR_MEM_DEBUG
            // TODO: DEBUGGING ONLY!
            if (tuples_in_triggerset_hash.contains(tuple_to_delete)) {
                std::cerr << "tuples_in_triggerset_hash(" << *tuple_to_delete << ").size() = " << tuples_in_triggerset_hash.at(tuple_to_delete).size() << std::endl;
                throw std::runtime_error(
                        std::format("DELETE ERRROR: Found {0} @ {1:x} in tuples_in_triggerset_hash key-set!",
                                    *tuple_to_delete, (size_t) tuple_to_delete));
            }

            for (auto const &[key_tup, value_tups]: tuple_provenance) {
                if (key_tup == tuple_to_delete) {
                    throw std::runtime_error(
                            std::format("DELETE ERRROR: Found {0} in tuple_provenance key!", *key_tup));
                }

                for (auto valueTup: value_tups) {
                    if (valueTup == tuple_to_delete) {
                        throw std::runtime_error(
                                std::format("DELETE ERRROR: Found {0} in tuple_provenance value set (for key {1})!",
                                            *valueTup, *key_tup));
                    }
                }
            }

            for (auto const &[h, tups]: tuple_handler_provenance) {
                for (auto t: tups) {
                    if (t == tuple_to_delete) {
                        throw std::runtime_error(
                                std::format("DELETE ERRROR: Found {0} in tuple_handler_provenance value set!", *t));
                    }
                }
            }

            // TODO: END DEBUGGING ONLY!
#endif

            db.remove_tuple(tuple_to_delete);
        }

        tuples_to_remove.clear();
    }

    void Reactor::tuples_triggered_handler_insert(ReactorSet<const Tuple *>::type from, const Handler *to) {
        size_t triggerset_hash = boost::hash_value(from);
        for (auto t: from) {
#ifdef REACTOR_TRACE
            std::cout << "SET INSERT: " << *t << " @ " << t << " Triggered Handler Insert (hash=" << triggerset_hash << ")"
                      << std::endl;
#endif
            rm_set_insert(tuples_in_triggerset_hash, t, triggerset_hash);
        }

        rm_set_insert(triggered_handlers_by_triggerset_hash, triggerset_hash, to);
    }

    bool Reactor::did_tuples_trigger_handler(ReactorSet<const Tuple *>::type from, const Handler *to) {
        size_t triggerset_hash = boost::hash_value(from);

        return triggered_handlers_by_triggerset_hash.contains(triggerset_hash) &&
               triggered_handlers_by_triggerset_hash.at(triggerset_hash).contains(to);
    }

    void Reactor::remove(const TupleNoun *subject, const TupleNoun *predicate, const TupleNoun *object) {
        remove(Tuple::mk(subject, predicate, object));
    }

    ReactorSet<const Tuple *>::type Reactor::query(const TupleNoun *subject, const TupleNoun *predicate,
                                                   const TupleNoun *object) {
        db.inc_tuple_noun(subject);
        db.inc_tuple_noun(predicate);
        db.inc_tuple_noun(object);

        std::lock_guard<std::mutex> guard(tupleMutex);
        ReactorSet<const Tuple *>::type result_tuples{};

        for (auto t: db.get_tuples()) {
            if(tuples_to_remove.contains(t)) { continue; }

            if (!subject->is_query() && *t->getSubject() != *subject) { continue; }

            if (!predicate->is_query() && *t->getPredicate() != *predicate) { continue; }

            if (!object->is_query() && *t->getObject() != *object) { continue; }

            result_tuples.insert(t);
        }

        db.dec_tuple_noun(subject);
        db.dec_tuple_noun(predicate);
        db.dec_tuple_noun(object);
        return result_tuples;
    }

}// namespace foxtalk