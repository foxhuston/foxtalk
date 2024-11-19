cmake_minimum_required(VERSION 3.25)
project(foxtalk_utils CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS Yes)

include(FetchContent)

################################################################################
FetchContent_Declare(
        argparse
        GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)

FetchContent_Declare(
        colorm
        GIT_REPOSITORY https://github.com/soreja/colorm.git
        GIT_TAG 4d41b6e0fbc32b3237e7e7171850e5f461368050
)
FetchContent_MakeAvailable(colorm)

# FetchContent_Declare(
#         color
#         GIT_REPOSITORY https://github.com/dmilos/color.git
# )
# FetchContent_MakeAvailable(color)

################################################################################

find_package(podofo REQUIRED)
find_package(argparse REQUIRED)

add_executable(pdf_identify pdf_identify.cpp)
target_link_libraries(pdf_identify podofo)
target_include_directories(pdf_identify PRIVATE "${colorm_SOURCE_DIR}")
