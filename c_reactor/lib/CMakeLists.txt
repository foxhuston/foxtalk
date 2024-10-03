set(CMAKE_CXX_STANDARD 26)

find_package(Boost REQUIRED)

add_library(
        reactor STATIC
        Reactor.cpp
        Reactor.h
        Tuple.h
        Db.cpp
        Db.h
        Symbol.h
        SharedObjectHandler.h
        Handler.h
        Tuple.cpp
        ReactorSet.h
        TupleNoun.h
        TupleNoun.cpp
)

target_include_directories(reactor PRIVATE
        ${Boost_INCLUDE_DIRS}
        ../vendor/gc-8.2.4/include
)

target_link_libraries(reactor PRIVATE gc dl)