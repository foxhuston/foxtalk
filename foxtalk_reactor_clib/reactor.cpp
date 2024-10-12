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

inline Triple from_flat_tuple(std::shared_ptr<kuzu::processor::FlatTuple> next) {
    auto const subj = kuzu::common::NodeVal::getProperties(next->getValue(0));
    auto const pred = kuzu::common::RelVal::getProperties(next->getValue(1));
    auto const obj = kuzu::common::NodeVal::getProperties(next->getValue(2));

    return Triple{triple_noun_from_kuzu_values(subj),
                  triple_noun_from_kuzu_values(pred),
                  triple_noun_from_kuzu_values(obj)};
}

void Reactor::claim(Triple t) {
    auto create_triple_cypher = foxtalk::reactor::cypher_gen::store_triple_cypher(t);
    auto res = connection->query(create_triple_cypher);
    if (!res->isSuccess()) {
        // fail sorry
    }
}

void Reactor::remove(Triple t) {
    // This... might be too much. Let's think about this

//    std::stringstream builder{};
//    builder << foxtalk::reactor::cypher_gen::match_for_triples_cypher(t);
//    builder << " DETACH DELETE subj, obj, pred;"; // Can we detach delete multiple like this?
//    auto delete_query = builder.str()
}

extern "C" {
// Should only be called through `tick()`
void foxtalk_claim(HandlerFunctionEnvironment *env) {
    assert(env->handler != nullptr);
    assert(env->reactor != nullptr);

    auto [t, bytes_read] = Triple::read_from_buffer(env->handler->handler_ipc_buffer, 0);
    // Record the actual claim
    env->reactor->claim(std::move(t));
    // Record a handler-provenance triple

    if (env->current_result != nullptr) {
        // Record a tuple-provenance triple
    }
}

void foxtalk_remove(HandlerFunctionEnvironment *) {}
bool foxtalk_getNextQueryResult(HandlerFunctionEnvironment *) { return false; }

}

void Reactor::tick() {
    auto handlers = getHandlers();
    for (auto h: getUninitializedHandlers(handlers)) {
        HandlerFunctionEnvironment hfe{
                nullptr, &h, this
        };

        h.init(&hfe);
    }

    for (auto current_handler: handlers) {
        auto res = connection->query(current_handler.cypher_query);
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
            "CREATE NODE TABLE Noun (type STRING, string_data STRING, int_data INT64, id SERIAL, PRIMARY KEY (id))");
    // assert(res->isSuccess());
    res = connection->query(
            "CREATE REL TABLE Predicate(FROM Noun TO Noun, type STRING, string_data STRING, int_data INT64)");
    // assert(res->isSuccess());
}

// Filters a list of handlers for those that are uninitialized
std::vector<Handler> Reactor::getUninitializedHandlers(const std::vector<Handler> &) {
    throw std::runtime_error("unimplemented!");
}

std::vector<Handler> Reactor::getHandlers() {
    std::unordered_map<std::string, Handler> out { };

    auto query = R"CYPHER(
         MATCH (handler:Noun)-[pred: Predicate { type: "Symbol" }]->(obj: Noun)
         WHERE EXISTS { MATCH (handler)-[u: Predicate{string_data: "is a"}]->(c: Noun{string_data: "handler"} ) }
         RETURN handler, pred, obj;
         )CYPHER";

    auto handler_query_res = connection->query(query);

//    Handler h {
//            .cypher_query = ""s,
//            .isAggregating = false,
//
//            .handler_ipc_buffer = nullptr,
//            .init = nullptr,
//            .handle = nullptr,
//            .freeTuple = nullptr,
//            .teardown = nullptr
//    };

    while (handler_query_res->hasNext()) {
        auto result = from_flat_tuple(handler_query_res->getNext());
        std::optional<std::string> subject = result.get_subject<std::string>();
        std::optional<std::string> pred = result.get_predicate<std::string>();

        assert(subject.has_value());
        assert(pred.has_value());

        if(!out.contains(subject.value())) {
            out.insert({ subject.value(), Handler {} });
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
            out[subject.value()].handler_ipc_buffer = reinterpret_cast<uint8_t *>(*obj);
        } else if (pred.value() == "has query") {
            if(auto obj = result.get_object<std::string>(); obj.has_value()) {
                out[subject.value()].cypher_query = *obj;
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
