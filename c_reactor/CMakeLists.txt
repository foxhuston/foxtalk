cmake_minimum_required(VERSION 3.29)
project(reactor)

set(CMAKE_CXX_STANDARD 26)

option(enable_cplusplus "C++ support" ON)

include_directories(lib/)
include_directories(vendor/gc-8.2.4/include)

add_subdirectory(vendor/gc-8.2.4)
add_subdirectory(lib)
add_subdirectory(tests)

find_package(Boost REQUIRED)

add_executable(reactor_run main.cpp FsWatcher.h)
target_link_libraries(reactor_run PUBLIC reactor)