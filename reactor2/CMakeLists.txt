cmake_minimum_required(VERSION 3.15)
project(foxtalk_tests C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

set(CMAKE_C_STANDARD 23)

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall")

add_subdirectory(database)

include(FetchContent)
FetchContent_Declare(
        googletest
        # Specify the commit you depend on and update it regularly.
        URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Now simply link against gtest or gtest_main as needed. Eg
include_directories(database)
add_executable(db_test db_test.cpp symbol_test.cpp
        reactor_test.cpp
        TestTupleNouns.h
)
target_link_libraries(db_test reactor2 gtest_main)
#add_test(NAME example_test COMMAND example)

