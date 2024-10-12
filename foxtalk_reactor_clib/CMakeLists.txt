cmake_minimum_required(VERSION 3.29)
project(reactor)

set(CMAKE_CXX_STANDARD 26)

include(FetchContent)
FetchContent_Declare(
        googletest
        # Specify the commit you depend on and update it regularly
        URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Now simply link against gtest or gtest_main as needed. Eg
include_directories(include/)

add_executable(tuple_test triple_tests.cpp debug_utils.cpp)
target_link_libraries(tuple_test gtest_main)


# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)


FetchContent_Declare(
        kuzulib
        URL https://github.com/kuzudb/kuzu/releases/download/v0.6.0/libkuzu-linux-x86_64.tar.gz
)

FetchContent_MakeAvailable(kuzulib)

set(KUZULIB_SO_PATH "${kuzulib_SOURCE_DIR}/libkuzu.so")  # Adjust the path accordingly

add_executable(db_test
        foxtalk_db.cpp
        foxtalk_db.h
        fs_watch_handler.cpp
        reactor.cpp
        include/reactor.h
        include/foxtalk_triple.h
        include/debug_utils.h
        include/foxtalk_handler_api_fns.h
        foxtalk_triple.cpp
        debug_utils.cpp)

target_link_libraries(db_test gtest_main ${KUZULIB_SO_PATH})

## Foxtalk dload handler
add_library(foxtalk_dload_handler dload_handler.cpp)

## Going to use this for testing handlers.
add_executable(handler_test handler_tests.cpp)
target_link_libraries(handler_test gtest_main foxtalk_dload_handler)

#target_link_libraries(db_test gtest_main)

#add_library(reactor include/foxtalk_handler.h)
#add_executable(reactor include/foxtalk_handler.h
#        tuple_tests.cpp)

add_executable(
        benchmark
        db_benchmark.cpp
        foxtalk_db.h
        include/foxtalk_triple.h
        vendor/ctrack.hpp)

target_link_libraries(benchmark ${KUZULIB_SO_PATH})