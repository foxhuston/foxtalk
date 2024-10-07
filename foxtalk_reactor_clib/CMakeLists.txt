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

add_executable(tuple_test triple_tests.cpp)
target_link_libraries(tuple_test gtest_main)

#add_library(reactor include/foxtalk_handler.h)
#add_executable(reactor include/foxtalk_handler.h
#        tuple_tests.cpp)
