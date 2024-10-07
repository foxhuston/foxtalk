cmake_minimum_required(VERSION 3.29)
project(reactor)

set(CMAKE_CXX_STANDARD 26)


include_directories(include/)
#add_library(reactor main.cpp include/foxtalk_handler.h)
add_executable(reactor main.cpp include/foxtalk_handler.h)
