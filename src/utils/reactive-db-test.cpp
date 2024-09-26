//
// Created by fox on 9/26/24.
//
#include <iostream>

#include "../reactive_db/ReactiveDb.h"

int main() {
    ReactiveDb db {};
    string lexi = "lexi";
    string fox = "fox";
    string husky = "husky";
    string demon_fox = "demon fox";
    string blue = "blue";

    db.claim(&lexi, "is a", &husky);
    db.claim(&fox, "is a", &demon_fox);


    // When (you) is a husky...
    auto allTheHuskies = db.query(nullopt, "is a", &husky);
    for(const auto& [who, isa, hoosk] : allTheHuskies) {
        // (you) are highlighted blue
        db.claim(who, "is highlighted", &blue);
    }

    // When (you) are highlighted (a color)...
    auto highlights = db.query(nullopt, "is highlighted", nullopt);
    for(const auto& [who, _, color] : highlights) {
        std::cout << "Going to highlight "
                  << *static_cast<string *>(who)
                  << " " << *static_cast<string *>(color)
                  << std::endl;
    }
}
