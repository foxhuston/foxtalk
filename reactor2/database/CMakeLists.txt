cmake_minimum_required(VERSION 3.15)
project(foxtalk_database C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_FLAGS_DEBUG_INIT "-Wall")

add_library(reactor2 STATIC
        database.c
        symbol.c
        reactor.c
)

target_include_directories(reactor2 PUBLIC include/)
