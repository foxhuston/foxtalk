//
// Created by fox on 9/26/24.
//

#ifndef FOXTALK_SYMBOL_H
#define FOXTALK_SYMBOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

typedef struct Symbol_t {
    char *mySymbol;
    struct Symbol_t *nextSymbols[];
} Symbol;

Symbol *intern_loop(char *orig, const char *current);

Symbol *intern(const char *str);

#ifdef __cplusplus
}
#endif

#endif //FOXTALK_SYMBOL_H