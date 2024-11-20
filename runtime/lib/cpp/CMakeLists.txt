cmake_minimum_required(VERSION 3.25)
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

##### DEBUG UTILS SHARED LIB #####
add_library(debug_utils SHARED debug_utils.cpp)

##### TRIPLE TEST #####
include_directories(include/)
add_executable(tuple_test tuple_tests.cpp debug_utils.cpp
        include/foxtalk_handler.h
        include/foxtalk_handler.hpp)
target_link_libraries(tuple_test gtest_main)

##### HANDLER TEST #####
include_directories(include/)
add_executable(handler_test handler_tests.cpp debug_utils.cpp
        include/foxtalk_handler.hpp
        include/foxtalk_handler.hpp
        example_handlers/example_tests.cpp)
target_link_libraries(handler_test gtest_main)

##### EXAMPLE HANDLERS #####
add_subdirectory(example_handlers)

