cmake_minimum_required(VERSION 3.15)
project(foxtalk CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(CMAKE_CXX_STANDARD 23)

find_package(OpenCV REQUIRED)
find_package(Vulkan REQUIRED COMPONENTS glslc)
# find_package(vk-bootstrap CONFIG REQUIRED)
## TODO: Optional, once I move to the "just use the display directly"
##       magic vulkan code.
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
# find_package(VulkanMemoryAllocator CONFIG REQUIRED)
# find_package(assimp CONFIG REQUIRED)
find_package(Freetype REQUIRED)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/src/shaders/)

################################################################################
## From https://stackoverflow.com/a/60472877

find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

function(compile_shader target)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "ENV;FORMAT" "SOURCES")
    foreach (source ${arg_SOURCES})
        add_custom_command(
                OUTPUT ${source}.${arg_FORMAT}
                DEPENDS ${source}
                DEPFILE ${source}.d
                COMMAND
                ${glslc_executable}
                $<$<BOOL:${arg_ENV}>:--target-env=${arg_ENV}>
                $<$<BOOL:${arg_FORMAT}>:-mfmt=${arg_FORMAT}>
                -MD -MF ${source}.d
                -o ${source}.${arg_FORMAT}
                ${CMAKE_CURRENT_SOURCE_DIR}/${source}
        )
        target_sources(${target} PRIVATE ${source}.${arg_FORMAT})
    endforeach ()
endfunction()

##### DYNAMIC LIBS #############################################################

add_library(image_proc SHARED src/CvProcessor.cpp)
add_library(dot_sandbox SHARED src/foxtalk_handlers/CvDotSandbox.cpp
        src/VImage.h
        src/VulkUtils.h
        src/VBuffer.h)

##### UTILS ####################################################################

find_package(podofo REQUIRED)

# From the argparse github. Don't think I like this very much...
include(FetchContent)
FetchContent_Declare(
        argparse
        GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)

add_executable(pdf-identify src/utils/pdf_identify.cpp)
target_link_libraries(pdf-identify argparse podofo)

################################################################################

add_subdirectory(c_reactor/lib)

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall")

#TODO: This is probably not the way to do it.
include_directories(
        ${OpenCV_INCLUDE_DIRS}
        ${FREETYPE_INCLUDE_DIRS}
        c_reactor/
        c_reactor/lib
        c_reactor/vendor/gc-8.2.4/include # TODO: Get rid of this
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")

add_executable(foxtalk
        src/main.cpp
        src/VulkUtils.h
        src/VBuffer.h
        src/VImage.h
        src/boot/BuildAndNotify.cpp
        src/boot/BuildAndNotify.h
)

compile_shader(foxtalk
        ENV vulkan
        FORMAT bin
        SOURCES
        src/shaders/simple.vert
        src/shaders/simple.frag
)


target_link_libraries(
        foxtalk

        PRIVATE
        ${OpenCV_LIBS}
        ${FREETYPE_LIBRARIES}
        Vulkan::Vulkan glfw glm::glm
        Vulkan::Headers

        reactor
)
