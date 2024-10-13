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

void Reactor::claim(const Triple& t) const
{
    const auto create_triple_cypher = store_triple_cypher(t);
    if (auto res = connection->query(create_triple_cypher); !res->isSuccess()) {
        // TODO: Add real logger and log this instead of throw
        throw std::runtime_error(res->getErrorMessage());
    }
}

void Reactor::remove(Triple t) {
    // This... might be too much. Let's think about this

//    std::stringstream builder{};
//    builder << foxtalk::reactor::cypher_gen::match_for_triples_cypher(t);
//    builder << " DETACH DELETE subj, obj, pred;"; // Can we detach delete multiple like this?
//    auto delete_query = builder.str()
}

inline void foxtalk_claim_internal(HandlerFunctionEnvironment *env, Triple& t)
{
    // Record the actual claim
    env->reactor->claim(t);
    // TODO Record a handler-provenance triple

    if (env->current_result != nullptr) {
        // TODO Record a tuple-provenance triple
    }
}

extern "C" {
    // Should only be called through `tick()`
    void foxtalk_claim(HandlerFunctionEnvironment *env)
    {
        assert(env->handler != nullptr);
        assert(env->reactor != nullptr);

        auto [t, bytes_read] = Triple::read_from_buffer(env->handler->handler_ipc_buffer, 0);
        foxtalk_claim_internal(env, t);
        env->reactor->claim(t);

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
        foxtalk_claim_internal(env, query_triple);

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
        } else {
            while (res->hasNext()) {
                auto handler_result = from_flat_tuple(res->getNext());
                handler_result.write_to_buffer(current_handler.handler_ipc_buffer, 0);

                HandlerFunctionEnvironment hfe{
                        &handler_result, &current_handler, this
                };

                current_handler.handle(&hfe);
            }
        }
    }
}

Reactor::Reactor(std::shared_ptr<Database> db) : database{std::move(db)} {
    connection = std::make_unique<Connection>(database.get());
    auto res = connection->query(
            "CREATE NODE TABLE IF NOT EXISTS Noun (type STRING, string_data STRING, int_data INT64, id SERIAL, PRIMARY KEY (id))");
    if (!res->isSuccess())
    {
        throw std::runtime_error(res->getErrorMessage());
    }
    res = connection->query(
            "CREATE REL TABLE IF NOT EXISTS Predicate(FROM Noun TO Noun, type STRING, string_data STRING, int_data INT64)");
    if (!res->isSuccess())
    {
        throw std::runtime_error(res->getErrorMessage());
    }
}

// lexi: Should we do this, or should we just return unitialized?
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
        out_vector.push_back(std::move(v));
    }
    return out_vector;
}
