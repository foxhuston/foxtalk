//
// Created by lexi on 10/10/24.
//

#ifndef FOXTALK_DB_H
#define FOXTALK_DB_H
#include <foxtalk_triple.h>
#include <sstream>
#include <format>

class foxtalk_db
{
private:
    std::unique_ptr<Database> database;

public:
    foxtalk_db(SystemConfig config)
    {
        database = std::make_unique<Database>(":memory:", config);

        auto connection = std::make_unique<Connection>(database.get());

        // auto res = connection->query("CREATE NODE TABLE Noun (type STRING, string_data STRING, int_data INT64, id SERIAL PRIMARY KEY (id))");
        auto res = connection->query("CREATE NODE TABLE Symbol (string_data STRING, PRIMARY KEY (string_data))");

        assert(res->isSuccess());
        res = connection->query("CREATE NODE TABLE CPtr (int_data INT64, PRIMARY KEY (int_data))");
        assert(res->isSuccess());
        res = connection->query("CREATE NODE TABLE U64 (int_data INT64, PRIMARY KEY (int_data))");
        assert(res->isSuccess());
        res = connection->query("CREATE NODE TABLE I64 (int_data INT64, PRIMARY KEY (int_data))");
        assert(res->isSuccess());

        res = connection->query("CREATE REL TABLE GROUP SymbolPredicate ("
                                "FROM Symbol to Symbol, "
                                "FROM Symbol to CPtr, "
                                "FROM Symbol to U64, "
                                "FROM Symbol to I64, "
                                "FROM CPtr To Symbol, "
                                "FROM CPtr to CPtr, "
                                "FROM CPtr to U64, "
                                "FROM CPtr to I64, "
                                "FROM U64 To Symbol, "
                                "FROM U64 to CPtr, "
                                "FROM U64 to U64, "
                                "FROM U64 to I64, "
                                "FROM I64 To Symbol, "
                                "FROM I64 to CPtr, "
                                "FROM I64 to U64, "
                                "FROM I64 to I64, "
                                "string_data STRING, int_data INT64)"); // Symbols aren't the only predicates... but for now, let's make it that way.

        assert(res->isSuccess());
    }

    std::string cypher_node_data(TripleNoun *noun)
    {
        std::stringstream builder{};
        switch (noun->type)
        {
        case TripleNoun::NounType::Query:
            break;
        case TripleNoun::NounType::Symbol:
            builder << "{string_data: \"" << std::get<std::string>(noun->data) << "\"}";
            break;
        case TripleNoun::NounType::CPtr:
        {
            void *ptr = std::get<void *>(noun->data);
            auto ptr_location = reinterpret_cast<uint64_t>(ptr);
            builder << "{int_data: " << ptr_location << "}";
            break;
        }
        case TripleNoun::NounType::U64:
            builder << "{int_data: " << std::get<uint64_t>(noun->data) << "}";
            break;
        case TripleNoun::NounType::I64:
            builder << "{int_data: " << std::get<int64_t>(noun->data) << "}";
            break;
        case TripleNoun::NounType::MAX:
            // should throw? Return error?
            break;
        }
        return builder.str();
    }

    std::string cypher_node_binding_and_type(TripleNoun *noun, std::string binding)
    {
        std::stringstream builder{};
        builder << binding << ":";
        switch (noun->type)
        {
        case TripleNoun::NounType::Query:
            builder << "Symbol:CPtr:U64:I64";
            break;
        case TripleNoun::NounType::Symbol:
            builder << "Symbol";
            break;
        case TripleNoun::NounType::CPtr:
            builder << "CPtr";
            break;
        case TripleNoun::NounType::U64:
            builder << "U64";
            break;
        case TripleNoun::NounType::I64:
            builder << "I64";
            break;
        case TripleNoun::NounType::MAX:
            // should throw? Return error?
            break;
        }
        return builder.str();
    }

    void store_triple(Triple *triple)
    {
        auto connection = std::make_unique<Connection>(database.get());

        std::stringstream builder{};

        /*
        MERGE (:User {name:'A'})-[:Follows]->(:User {name:'B'})-[:LivesIn]->(:City {name:'Toronto'});
        MATCH (a:User)-[:Follows]->(b:User)-[:LivesIn]->(c:City)
        RETURN a.name, b.name, c.name;
        */

        builder << "MERGE(";
        builder << cypher_node_binding_and_type(&triple->subject_, "");
        builder << cypher_node_data(&triple->subject_);
        builder << ")-[";
        builder << cypher_node_binding_and_type(&triple->predicate_, "");
        builder << "Predicate";
        builder << cypher_node_data(&triple->predicate_);
        builder << "]->(";
        builder << cypher_node_binding_and_type(&triple->object_, "");
        builder << cypher_node_data(&triple->object_);
        builder << ");";

        // builder << "MATCH (";
        // builder << cypher_node_binding_and_type(&triple->subject_, "subj");
        // builder << ")-[";
        // builder << cypher_node_binding_and_type(&triple->predicate_, "pred");
        // builder << "Predicate]->(";
        // builder << cypher_node_binding_and_type(&triple->object_, "obj");
        // builder << ");\n";
        //
        // builder << "RETURN subj.data, pred.data, obj.data;";

        auto query = builder.str();
        auto res = connection->query(query);
        assert(res->isSuccess());
        std::cout << res->getQuerySummary();
    }

    TripleNoun triple_noun_from_kuzu_values(const std::string &noun_type, kuzu::common::Value *data)
    {

        if (noun_type == "SymbolPredicate_Symbol_Symbol")
        {
            return TripleNoun{kuzu::common::RelVal::getPropertyVal(data, 0)->getValue<std::string>()};
        }
        if (noun_type == "Symbol")
        {
            return TripleNoun{kuzu::common::NodeVal::getPropertyVal(data, 0)->getValue<std::string>()};
        }
        if (noun_type == "CPtr")
        {
            return TripleNoun{reinterpret_cast<void *>(kuzu::common::NodeVal::getPropertyVal(data, 1)->getValue<int64_t>())};
        }
        if (noun_type == "U64")
        {
            return TripleNoun{static_cast<uint64_t>(kuzu::common::NodeVal::getPropertyVal(data, 1)->getValue<int64_t>())};
        }
        if (noun_type == "I64")
        {
            return TripleNoun{kuzu::common::NodeVal::getPropertyVal(data, 1)->getValue<int64_t>()};
        }
        throw std::runtime_error(std::format("Noun type {} invalid", noun_type));
    }

    std::vector<Triple> get_triples(Triple *triple)
    {
        auto connection = std::make_unique<Connection>(database.get());

        std::stringstream builder{};
        builder << "MATCH(";
        builder << cypher_node_binding_and_type(&triple->subject_, "subj");
        builder << cypher_node_data(&triple->subject_);
        builder << ")-[";
        builder << cypher_node_binding_and_type(&triple->predicate_, "pred");
        builder << "Predicate";
        builder << cypher_node_data(&triple->predicate_);
        builder << "]->(";
        builder << cypher_node_binding_and_type(&triple->object_, "obj");
        builder << cypher_node_data(&triple->object_);
        builder << ") ";
        builder << "RETURN subj, pred, obj;";

        auto query = builder.str();
        auto res = connection->query(query);
        std::cout << res->getQuerySummary();

        std::vector<Triple> results{};

        while (res->hasNext())
        {
            auto next = res->getNext();
            auto subj = next->getValue(0);
            auto pred = next->getValue(1);
            auto obj = next->getValue(2);

            auto subj_type = kuzu::common::NodeVal::getLabelVal(subj)->getValue<std::string>();
            auto pred_type = kuzu::common::RelVal::getLabelVal(pred)->getValue<std::string>();
            auto obj_type = kuzu::common::NodeVal::getLabelVal(obj)->getValue<std::string>();

            results.push_back({triple_noun_from_kuzu_values(subj_type, subj),
                               triple_noun_from_kuzu_values(pred_type, pred),
                               triple_noun_from_kuzu_values(obj_type, obj)});
        }
        return results;
    }
};

#endif // FOXTALK_DB_H
