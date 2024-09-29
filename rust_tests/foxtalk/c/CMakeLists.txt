cmake_minimum_required(VERSION 3.15)
project(foxtalk_clibs CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(CMAKE_CXX_STANDARD 23)

include_directories(../reactor/c/)

add_library(camera_format_handler CameraFormatHandler.cpp)
add_library(camera_pixel_handler CameraPixelHandler.cpp)
