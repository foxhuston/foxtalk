//
// Created by fox on 9/26/24.
//

#ifndef FOXTALK_SYMBOL_H
#define FOXTALK_SYMBOL_H

#include <stdlib.h>

typedef struct Symbol_t {
    char *mySymbol;
    struct Symbol *nextSymbols[];
} Symbol;

Symbol* intern_loop(char* orig, const char* current);
Symbol *intern(const char* str);

#endif //FOXTALK_SYMBOL_H