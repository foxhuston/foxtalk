//
// Created by lexi on 10/10/24.
//

#ifndef FOXTALK_DB_H
#define FOXTALK_DB_H

#include "foxtalk_triple.h"


class foxtalk_db {
private:
    std::unique_ptr<Database> database;

public:
    foxtalk_db(SystemConfig config)
    {
        database = std::make_unique<Database>(":memory:", config);

        auto connection = std::make_unique<Connection>(database.get());
        connection->query("CREATE NODE TABLE Symbol (data STRING) PRIMARY KEY (data))");
//        connection->query("CREATE NODE TABLE Symbol (data STRING) PRIMARY KEY (data))");
        connection->query("CREATE NODE TABLE CPtr (data INT64) PRIMARY KEY (data))");
        connection->query("CREATE NODE TABLE U64 (data INT64) PRIMARY KEY (data))");
        connection->query("CREATE NODE TABLE I64 (data INT64) PRIMARY KEY (data))");


        connection->query("CREATE REL TABLE GROUP SymbolPredicate ("
                          "FROM Symbol To Symbol, "
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
                          "data STRING)");
        // Symbols aren't the only predicates... but for now, let's make it that way.
    }

    std::string node_cypher(TripleNoun* noun)
    {
        switch (noun->type)
        {
        }
    }

    void store_triple(Triple* triple)
    {
        auto connection = std::make_unique<Connection>(database.get());


    }

    std::vector<Triple*> get_triples(Triple* query)
    {

    }


};



#endif //FOXTALK_DB_H
