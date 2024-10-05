#include "symbol.h"

Symbol *intern(const char* str) {
    return intern_loop(str, str);
}

Symbol* intern_loop(char* orig, const char* current) {
    exit(-1); // TODO: Unimplemented


//    if(*current != 0) {
//        return nextSymbols[*current].intern(orig, current+1);
//    } else {
//        mySymbol = orig;
//        return this;
//    }
}
