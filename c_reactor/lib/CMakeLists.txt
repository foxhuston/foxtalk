set(CMAKE_CXX_STANDARD 26)

find_package(Boost REQUIRED)

# N.B.: This feels like it should be static, but if it is, dlopen can't find
# symbols. I got tired of dealing with it, so I'll just have to manually link
# gc and reactor together whenever I need to make a new project -_-
add_library(
        reactor
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

target_link_libraries(reactor gc dl)