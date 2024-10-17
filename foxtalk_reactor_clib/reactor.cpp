//
// Created by fox on 10/11/24.
//

#include <cassert>
#include <string>
#include <unordered_map>
#include "vendor/kuzu.hpp"
#include "foxtalk_triple.h"

#include "cypher_gen.h"
#include "reactor.h"

using namespace std::literals;
using namespace kuzu::main;
using namespace foxtalk::reactor::cypher_gen;

inline Triple from_flat_tuple(const std::shared_ptr<kuzu::processor::FlatTuple>& next) {
    auto const subj = kuzu::common::NodeVal::getProperties(next->getValue(0));
    auto const pred = kuzu::common::RelVal::getProperties(next->getValue(1));
    auto const obj = kuzu::common::NodeVal::getProperties(next->getValue(2));

    return Triple{triple_noun_from_kuzu_values(subj),
                  triple_noun_from_kuzu_values(pred),
                  triple_noun_from_kuzu_values(obj)};
}

void Reactor::claim(Triple t, std::optional<std::string> creating_handler_name) const
{
    const auto create_triple_cypher = store_triple_cypher(t, creating_handler_name);
    if (auto res = connection->query(create_triple_cypher); !res->isSuccess()) {
        // TODO: Add real logger and log this instead of throw
        throw std::runtime_error(res->getErrorMessage());
    }
    if (creating_handler_name.has_value())
    {
        std::string key = creating_handler_name.value();
        auto set = current_tick_created_triples.at(key);
        auto res = set.insert(std::move(t));
        if (!res.second)
        {
            std::cerr << "Claim failed!? ";
        }
    }
}

void Reactor::remove(Triple t) {
    /*
     * Right now, this leaves the nouns dangling, as we only ever delete the predicate.
     * The predicate is the edge of the graph, so that's what actually represents "the triple", uniquely.
     * This should be totally safe, since even if later a predicate attaches to a previously detatched
     * node, it's the predicate that defines that new triple, and that predicate will be new.
     *
     * The downside is that we'll have a bit of a messy db to deal with. We won't want to keep this for
     * very long, but as a start, this is fine. We also might not even use Kuzu long-term, so I don't
     * want to optimize for this sort of thing yet... until we validate that it can do what we want, instead
     * of us just writing our own bespoke query engine.
     */

    std::stringstream builder{};
    builder << match_for_triples_cypher(t);
    builder << " DELETE pred";

    if (auto res = connection->query(builder.str()); !res->isSuccess())
    {
        throw std::runtime_error(res->getErrorMessage());
    }

}

inline void foxtalk_claim_internal(HandlerFunctionEnvironment *env, Triple t)
{
    // Record the actual claim
    env->reactor->claim(std::move(t), env->handler->name);
    // handler provenance is recorded in kuzu
    // if (env->current_result != nullptr) {
        // TODO Record a tuple-provenance triple
        // TODO V2: Tuple-provenance is now just done with ticks, I think?
    // }
}

extern "C" {
    // Should only be called through `tick()`
    void foxtalk_claim(HandlerFunctionEnvironment *env)
    {
        assert(env->handler != nullptr);
        assert(env->reactor != nullptr);

        auto [t, bytes_read] = Triple::read_from_buffer(env->handler->handler_ipc_buffer, 0);
        foxtalk_claim_internal(env, std::move(t));

    }

    void foxtalk_remove(HandlerFunctionEnvironment *) { /* TODO */ }
    bool foxtalk_getNextQueryResult(HandlerFunctionEnvironment *env) { return env->current_result != nullptr; }
    void foxtalk_registerHandleQuery(HandlerFunctionEnvironment *env)
    {
        assert(env->handler != nullptr);
        assert(env->reactor != nullptr);

        auto [t, bytes_read] = Triple::read_from_buffer(env->handler->handler_ipc_buffer, 0);
        auto query = query_for_triples_cypher(t);

        Triple query_triple = {{env->handler->name}, {"has query"s}, {query}};
        foxtalk_claim_internal(env, std::move(query_triple));

        // lexi: Should we do this, or just let the second tick do the work here?
        env->handler->cypher_query = query;
        // update: I don't think this actually changes the underlying handler in the map... we must be copying somewhere
    }
}


void Reactor::tick() {

    auto [initialized_handlers, uninitialized_handlers] = split_handlers_by_initialization_state(getHandlers());
    for (auto h: uninitialized_handlers) {
        HandlerFunctionEnvironment hfe{
                nullptr, &h, this
        };

        h.init(&hfe);
    }

    for (auto current_handler: initialized_handlers) {
        auto res = connection->query(current_handler.cypher_query.value());
        if (current_handler.isAggregating) {
            throw std::runtime_error("Unimplemented!");
        } else
        {
            while (res->hasNext())
            {
                auto handler_result = from_flat_tuple(res->getNext());
                handler_result.write_to_buffer(current_handler.handler_ipc_buffer, 0);

                HandlerFunctionEnvironment hfe{
                    &handler_result, &current_handler, this
                };

                current_handler.handle(&hfe);
            }
            auto this_handler_this_tick_created_triples = current_tick_created_triples.at(current_handler.name);
            auto this_handler_last_tick_created_triples = last_tick_created_triples.at(current_handler.name);


            for (auto &this_tick_triple: this_handler_this_tick_created_triples)
            {
                auto last_tick_found = this_handler_last_tick_created_triples.find(this_tick_triple);
                if (last_tick_found != this_handler_last_tick_created_triples.end())
                {
                    this_handler_last_tick_created_triples.erase(last_tick_found);
                    // Do I need to free the memory?
                }
            }

            // Now, the only things remaining in this_handler_last_tick_created_triples are things that need deleting
            // AKA things in D_t that aren't in D_(t-1)
            for (auto &triple_to_delete: this_handler_last_tick_created_triples)
            {
                // move the original i think??
                remove(std::move(triple_to_delete));
                std::cout << "Deleting... " << triple_to_delete;
            }
            this_handler_last_tick_created_triples.clear();
            this_handler_last_tick_created_triples.swap(this_handler_this_tick_created_triples);

        }
    }
}


Reactor::Reactor(std::shared_ptr<Database> db) : database{std::move(db)}
{
    connection = std::make_unique<Connection>(database.get());
    auto res = connection->query(
        "CREATE NODE TABLE IF NOT EXISTS Noun (type STRING, string_data STRING, int_data INT64, id SERIAL, PRIMARY KEY (id))");
    if (!res->isSuccess())
    {
        throw std::runtime_error(res->getErrorMessage());
    }
    res = connection->query(
        "CREATE REL TABLE IF NOT EXISTS Predicate(FROM Noun TO Noun, type STRING, string_data STRING, int_data INT64, has_been_handled_by STRING[] DEFAULT [])");
    if (!res->isSuccess())
    {
        throw std::runtime_error(res->getErrorMessage());
    }
}

std::pair<std::vector<Handler>, std::vector<Handler>> Reactor::split_handlers_by_initialization_state(const std::vector<Handler> &handlers)
{
    std::vector<Handler> init;
    std::vector<Handler> not_init;
    for (const auto& h: handlers)
    {
        if (h.cypher_query.has_value())
        {
            init.push_back(h);
        }
        else
        {
            not_init.push_back(h);
        }
    }
    return {init, not_init};
}


std::vector<Handler> Reactor::getHandlers() {
    std::unordered_map<std::string, Handler> out { };

    auto query = R"CYPHER(
         MATCH (handler:Noun)-[pred: Predicate]->(obj: Noun)
         WHERE EXISTS { MATCH (handler)-[u: Predicate{string_data: "is a"}]->(c: Noun{string_data: "handler"} ) }
         RETURN handler, pred, obj;
         )CYPHER";

    auto handler_query_res = connection->query(query);

    while (handler_query_res->hasNext()) {
        auto result = from_flat_tuple(handler_query_res->getNext());
        std::optional<std::string> subject = result.get_subject<std::string>();
        std::optional<std::string> pred = result.get_predicate<std::string>();

        assert(subject.has_value());
        assert(pred.has_value());

        if(!out.contains(subject.value())) {
            out.insert({ subject.value(), Handler { .name = subject.value() } });
        }

        if (pred.value() == "has init") {
            auto obj = result.get_object<void *>();
            assert(obj != nullptr);
            out[subject.value()].init = reinterpret_cast<Handler::Init>(*obj);
        } else if (pred.value() == "has handle") {
            auto obj = result.get_object<void *>();
            assert(obj != nullptr);
            out[subject.value()].handle = reinterpret_cast<Handler::Handle>(*obj);
        } else if (pred.value() == "has free tuple") {
            auto obj = result.get_object<void *>();
            assert(obj != nullptr);
            out[subject.value()].freeTuple = reinterpret_cast<Handler::FreeTuple>(*obj);
        } else if (pred.value() == "has teardown") {
            auto obj = result.get_object<void *>();
            assert(obj != nullptr);
            out[subject.value()].teardown = reinterpret_cast<Handler::Teardown>(*obj);
        } else if (pred.value() == "has ipc_buffer") {
            auto obj = result.get_object<void *>();
            assert(obj != nullptr);
            out[subject.value()].handler_ipc_buffer = static_cast<uint8_t *>(*obj);
        } else if (pred.value() == "has query") {
            if(auto obj = result.get_object<std::string>(); obj.has_value()) {
                out[subject.value()].cypher_query = obj;
            }
        }
        // TODO: handle isAggregating
    }

    // get the query that this handler cares about
    // query the db for that query
    // (if non-aggregating) for each of the results, call the handler's handle function
    // (if aggregating) call the handle function with all of the results (** in spirit **)
    std::vector<Handler> out_vector{};
    for(auto [k, v] : out) {

        auto cur_it = current_tick_created_triples.find(v.name);

        if( cur_it == current_tick_created_triples.end()) {
            current_tick_created_triples.insert(std::make_pair(v.name, std::unordered_set<Triple>()));
        }

        auto last_it = last_tick_created_triples.find(v.name);

        if( last_it == last_tick_created_triples.end()) {
            last_tick_created_triples.insert(std::make_pair(v.name, std::unordered_set<Triple>()));
        }
        out_vector.push_back(std::move(v));
    }
    return out_vector;
}
