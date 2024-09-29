cmake_minimum_required(VERSION 3.15)
project(test CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(CMAKE_CXX_STANDARD 23)

include_directories(test ../../../)

add_library(test test.cpp)
add_library(test_ids test_ids.cpp)