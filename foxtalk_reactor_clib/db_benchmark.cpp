#include <iostream>

#define CTRACK_DISABLE_EXECUTION_POLICY
#include <cypher_gen.h>

#include "reactor.h"
#include "vendor/ctrack.hpp"

#define FOXTALK_BENCHMARK
using namespace foxtalk::reactor::cypher_gen;
int main(int argc, char* argv[]) {

    uint64_t num_records = 100ul;
    uint64_t query_multiplicity = 10ul;
    if (argc == 3)
    {
        char* end;
        num_records = strtol(argv[1], &end, 10);
        query_multiplicity = strtol(argv[2], &end, 10);
    }

    std::cout << "Query multiplicity: " << query_multiplicity << std::endl;
    std::cout << "Create num records: " << num_records << std::endl;

    int cnt = 0;
    int qry = 0;
    uint64_t num_results = 0;

    kuzu::main::SystemConfig db_config {
        -1u,
        16,
        true,
        false,
        -1u,
        true,
        16777216 };
    auto db = std::make_shared<kuzu::main::Database>(":memory:", db_config);
    Reactor reactor { db };

    auto conn = std::make_unique<kuzu::main::Connection>(db.get());

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

        reactor.claim(a);
        cnt++;
        reactor.claim(b);
        cnt++;
        reactor.claim(c);
        cnt++;
        reactor.claim(d);
        cnt++;
        reactor.claim(e);
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
                auto results = conn->query(query_for_triples_cypher(query));
                qry++;
                num_results += results->getNumTuples();
                continue;
            }
            if (j%3 == 0)
            {
                auto query = Triple {
                    TripleNoun { i },
                    TripleNoun { },
                    TripleNoun { static_cast<void*>(&i_str)  }
                };
                auto results = conn->query(query_for_triples_cypher(query));
                qry++;
                num_results += results->getNumTuples();
                continue;
            }
            if (j%5 == 0)
            {
                auto query = Triple {
                    TripleNoun { },
                    TripleNoun { i_str },
                    TripleNoun { }
                };
                auto results = conn->query(query_for_triples_cypher(query));
                qry++;
                num_results += results->getNumTuples();
                continue;
            }
            auto query = Triple {
                TripleNoun { },
                TripleNoun { },
                TripleNoun { }
            };
            auto results = conn->query(query_for_triples_cypher(query));
            qry++;
            num_results += results->getNumTuples();

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
            auto results = conn->query(query_for_triples_cypher(query));
            qry++;
            num_results += results->getNumTuples();
            continue;
        }
        if (j%3 == 0)
        {
            auto query = Triple {
                TripleNoun { j },
                TripleNoun { },
                TripleNoun { static_cast<void*>(&i_str)  }
            };
            auto results = conn->query(query_for_triples_cypher(query));
            qry++;
            num_results += results->getNumTuples();
            continue;
        }
        if (j%5 == 0)
        {
            auto query = Triple {
                TripleNoun { },
                TripleNoun { i_str },
                TripleNoun { }
            };
            auto results = conn->query(query_for_triples_cypher(query));
            qry++;
            num_results += results->getNumTuples();
            continue;
        }
        auto query = Triple {
            TripleNoun { },
            TripleNoun { },
            TripleNoun { }
        };
        auto results = conn->query(query_for_triples_cypher(query));
        qry++;


        num_results += results->getNumTuples();

    }

    std::cout << "total number of queries run: " << qry << std::endl;
    std::cout << "total number of results returned: " << num_results << std::endl;

    ctrack::result_print();

}

// BEGIN HANDLERS FOR BENCHMARKS

// END HANDLERS FOR BENCHMARKS