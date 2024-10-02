set(CMAKE_CXX_STANDARD 26)

find_package (Boost REQUIRED)
include_directories (${Boost_INCLUDE_DIRS})

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
        ../DynamicHandler.h
)

target_link_libraries(reactor gc gccpp dl)

target_include_directories(reactor PRIVATE ../vendor/gc-8.2.4/include)
