#include <iostream>

#define CTRACK_DISABLE_EXECUTION_POLICY
#include "foxtalk_db.h"

#define FOXTALK_BENCHMARK
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

    int cnt = 0;
    int qry = 0;
    int num_results = 0;

    SystemConfig config;
    auto db = foxtalk_db(config);

    for (int i = 0; i < num_records/5; i++)
    {
        CTRACK;
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
        cnt++;
        db.store_triple(&b);
        cnt++;
        db.store_triple(&c);
        cnt++;
        db.store_triple(&d);
        cnt++;
        db.store_triple(&e);
        cnt++;

        for (auto j = 0; j < query_multiplicity; j++)
        {

            CTRACK;
            if (j%2 == 0)
            {
                auto query = Triple {
                    TripleNoun { i },
                    TripleNoun { },
                    TripleNoun { }
                };
                auto results = db.get_triples(&query);
                qry++;
                num_results += results.size();
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
                num_results += results.size();
                qry++;
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
                num_results += results.size();
                qry++;
                continue;
            }
            auto query = Triple {
                TripleNoun { },
                TripleNoun { },
                TripleNoun { }
            };
            auto results = db.get_triples(&query);
            num_results += results.size();
            qry++;

        }
    }

    std::cout << "total number of records added: " << cnt << std::endl;
    std::cout << "total number of queries run: " << qry << std::endl;
    std::cout << "total number of results returned: " << num_results << std::endl;

    // now query a ton of times

    std::cout << "Second stage..." << std::endl;
    qry = 0;
    num_results = 0;
    for (auto j = 0; j < num_records; j++)
    {
        auto i_str = std::to_string(j);
        CTRACK;
        if (j%2 == 0)
        {
            auto query = Triple {
                TripleNoun { j },
                TripleNoun { },
                TripleNoun { }
            };
            auto results = db.get_triples(&query);
            qry++;
            num_results += results.size();
            continue;
        }
        if (j%3 == 0)
        {
            auto query = Triple {
                TripleNoun { j },
                TripleNoun { },
                TripleNoun { static_cast<void*>(&i_str)  }
            };
            auto results = db.get_triples(&query);
            num_results += results.size();
            qry++;
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
            num_results += results.size();
            qry++;
            continue;
        }
        auto query = Triple {
            TripleNoun { },
            TripleNoun { },
            TripleNoun { }
        };
        auto results = db.get_triples(&query);
        num_results += results.size();
        qry++;

    }

    std::cout << "total number of queries run: " << qry << std::endl;
    std::cout << "total number of results returned: " << num_results << std::endl;

    ctrack::result_print();

}