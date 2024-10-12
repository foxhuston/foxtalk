#include <iostream>

#include "foxtalk_db.h"

//
// Created by lexi on 10/11/24.
//
int main(int argc, char* argv[]) {

    int num_records = 100;
    int query_multiplicity = 10;
    if (argc == 3)
    {
        num_records = atoi(argv[1]);
        query_multiplicity = atoi(argv[2]);
    }

    std::cout << "Query multiplicity: " << query_multiplicity << std::endl;
    std::cout << "Create num records: " << num_records << std::endl;

    SystemConfig config;
    auto db = foxtalk_db(config);

    for (int i = 0; i < num_records/5; i++)
    {
        auto i_str = std::to_string(i);
        auto a = Triple {
            TripleNoun { i_str },
            TripleNoun { i },
            TripleNoun { static_cast<void*>(&i_str) } };

        auto b = Triple {
            TripleNoun { static_cast<void*>(&i_str) },
            TripleNoun { i_str },
            TripleNoun { i } };

        auto c = Triple {
            TripleNoun { i },
            TripleNoun { i_str },
            TripleNoun { static_cast<void*>(&i_str) } };

        auto d = Triple {
            TripleNoun { static_cast<void*>(&i_str) },
            TripleNoun { i_str },
            TripleNoun { i } };

        auto e = Triple {
            TripleNoun { i },
            TripleNoun { static_cast<void*>(&i_str) },
            TripleNoun { i_str } };

        db.store_triple(&a);
        db.store_triple(&b);
        db.store_triple(&c);
        db.store_triple(&d);
        db.store_triple(&e);


        for (auto j = 0; j < query_multiplicity; j++)
        {
            if (j%2 == 0)
            {
                auto query = Triple {
                    TripleNoun { i },
                    TripleNoun { },
                    TripleNoun { }
                };
                auto results = db.get_triples(&query);
                continue;
            }
            if (j%3 == 0)
            {
                auto query = Triple {
                    TripleNoun { i },
                    TripleNoun { },
                    TripleNoun { static_cast<void*>(&i_str)  }
                };
                auto results = db.get_triples(&query);
                continue;
            }
            if (j%5 == 0)
            {
                auto query = Triple {
                    TripleNoun { },
                    TripleNoun { i_str },
                    TripleNoun { }
                };
                auto results = db.get_triples(&query);
                continue;
            }
            auto query = Triple {
                TripleNoun { },
                TripleNoun { },
                TripleNoun { }
            };
            auto results = db.get_triples(&query);


        }


    }
}