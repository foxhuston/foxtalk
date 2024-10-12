//
// Created by fox on 10/11/24.
//

#include "reactor.h"
#include <string>

#include "vendor/kuzu.hpp"
#include "foxtalk_triple.h"
#include "foxtalk_handler_api_fns.h"

#include "cypher_gen.h"

using namespace kuzu::main;

void Reactor::claim(Triple t) {
    auto create_triple_cypher = foxtalk::reactor::cypher_gen::store_triple_cypher(t);
    auto res = connection->query(create_triple_cypher);
    if (res->isSuccess()) {
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

void Reactor::tick() {
    // get all handlers
//    auto get_handler_query = foxtalk::reactor::cypher_gen::match_for_triples_cypher({{}, {"is a"s}, {"handler"s}});
//    auto handlers = connection->query(get_handler_query);
//
//    while (handlers->hasNext())
//    {
//        auto next = handlers->getNext();
//        auto const subj = kuzu::common::NodeVal::getProperties(next->getValue(0));
//        auto const pred = kuzu::common::RelVal::getProperties(next->getValue(1));
//        auto const obj = kuzu::common::NodeVal::getProperties(next->getValue(2));
//
////        auto handler = (triple_noun_from_kuzu_values(subj),
////                           triple_noun_from_kuzu_values(pred),
////                           triple_noun_from_kuzu_values(obj));
//    }
}

Reactor::Reactor(std::shared_ptr<Database> db) : database{ std::move(db) }
{
    connection = std::make_unique<Connection>(database.get());
    auto res = connection->query("CREATE NODE TABLE Noun (type STRING, string_data STRING, int_data INT64, id SERIAL, PRIMARY KEY (id))");
    // assert(res->isSuccess());
    res = connection->query("CREATE REL TABLE Predicate(FROM Noun TO Noun, type STRING, string_data STRING, int_data INT64)");
    // assert(res->isSuccess());
}
