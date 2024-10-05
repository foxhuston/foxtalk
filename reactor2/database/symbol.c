#include <string.h>
#include "include/symbol.h"

static Symbol *global_symbol_root = nullptr;

// ustr/ustring has this bonkers property that when you intern strings, you get (const char*)'s back that you can
// dereference, but you can *also* compare them by pointer equality. That is, intern("hello") == intern("hello") anywhere
// across the life of the program. This is nuts.

Symbol *malloc_symbol() {
    constexpr size_t sym_size = sizeof(Symbol) + 256 * sizeof(Symbol);

    auto new_ptr = (Symbol*) malloc(sym_size);
    memset(new_ptr, 0, sym_size);

    return new_ptr;
}

const Symbol *intern(const char* str) {
    if(global_symbol_root == nullptr) {
        global_symbol_root = malloc_symbol();
        global_symbol_root->str = "";
    }

    Symbol* out = global_symbol_root;
    const char* ptr = str;

    while(*ptr != 0) {
        if(out->nextSymbols[*ptr] == nullptr) {
            out->nextSymbols[*ptr] = malloc_symbol();

            size_t current_length = ptr - str + 1;
            out->nextSymbols[*ptr]->str = malloc(sizeof(char) * current_length + 1);
            strncpy(out->nextSymbols[*ptr]->str, str, current_length);
            out->nextSymbols[*ptr]->str[current_length] = 0;
        }

        out = out->nextSymbols[*ptr];
        ptr = ptr + 1;
    }

    return out;
}