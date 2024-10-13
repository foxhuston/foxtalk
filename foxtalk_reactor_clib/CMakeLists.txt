cmake_minimum_required(VERSION 3.29)
project(reactor)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Ofast")


##### GTEST #####
include(FetchContent)
FetchContent_Declare(
        googletest
        # Specify the commit you depend on and update it regularly
        URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

##### KUZU #####
FetchContent_Declare(
        kuzulib
        URL https://github.com/kuzudb/kuzu/releases/download/v0.6.0/libkuzu-linux-x86_64.tar.gz
)

FetchContent_MakeAvailable(kuzulib)
set(KUZULIB_SO_PATH "${kuzulib_SOURCE_DIR}/libkuzu.so")  # Adjust the path accordingly

##### REACTOR #####

include_directories(include/)
include_directories(./)

add_library(reactor
        reactor.cpp
        debug_utils.cpp
        cypher_gen.cpp
)

target_link_libraries(reactor PUBLIC ${KUZULIB_SO_PATH})


##### TRIPLE TEST #####
add_executable(triple_test triple_tests.cpp)
target_link_libraries(triple_test reactor gtest_main)


## Foxtalk dload handler
# add_library(foxtalk_dload_handler handlers/dload_handler.cpp)

## Going to use this for testing handlers.
add_executable(handler_test handler_tests.cpp)
target_link_libraries(handler_test reactor gtest_main ${KUZULIB_SO_PATH})

##### DB BENCHMARK #####
add_executable(
        benchmark
        db_benchmark.cpp
        vendor/ctrack.hpp
)

target_link_libraries(benchmark reactor ${KUZULIB_SO_PATH})