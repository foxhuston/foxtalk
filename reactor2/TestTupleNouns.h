//
// Created by fox on 10/5/24.
//

#ifndef FOXTALK_TESTS_TESTTUPLENOUNS_H
#define FOXTALK_TESTS_TESTTUPLENOUNS_H

#include "database.h"

static TupleNoun lexi = { .type = TupleNoun::Type::Sym, .data = { .symbol = intern("lexi") } };
static TupleNoun is = { .type = TupleNoun::Type::Sym, .data = { .symbol = intern("is") } };
static TupleNoun isA = { .type = TupleNoun::Type::Sym, .data = { .symbol = intern("is a") } };
static TupleNoun husky  = { .type = TupleNoun::Type::Sym, .data = { .symbol = intern("husky") } };
static TupleNoun cool  = { .type = TupleNoun::Type::Sym, .data = { .symbol = intern("cool") } };


#endif //FOXTALK_TESTS_TESTTUPLENOUNS_H
