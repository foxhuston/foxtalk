//
// Created by fox on 9/26/24.
//

#ifndef FOXTALK_SYMBOL_H
#define FOXTALK_SYMBOL_H

#include <string>
#include <iostream>


struct Symbol {
public:
    Symbol *intern(std::string str) {
        return intern(str, str.c_str());
    }

    friend std::ostream& operator<<(std::ostream& os, const Symbol& sym) {
        os << "#<" << sym.mySymbol << ">";
        return os;
    }

    Symbol() { }

    ~Symbol() {
//        if(nextSymbols != nullptr) {
//            delete nextSymbols;
//        }
    }

private:
    std::string mySymbol;

    Symbol* intern(std::string orig, const char* current) {
        if(nextSymbols == nullptr) {
            nextSymbols = new Symbol[255];
        }

        if(*current != 0) {
            return nextSymbols[*current].intern(orig, current+1);
        } else {
            mySymbol = orig;
            return this;
        }
    }

    // TODO: What's the actual number, here?
    Symbol *nextSymbols = nullptr;
};

#endif //FOXTALK_SYMBOL_H
