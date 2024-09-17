cmake_minimum_required(VERSION 3.15)
project(foxtalk CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(CXX_STANDARD C++23)

find_package(OpenCV REQUIRED)
find_package(Vulkan REQUIRED COMPONENTS glslc)
# find_package(vk-bootstrap CONFIG REQUIRED)
## TODO: Optional, once I move to the "just use the display dierctly"
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
    foreach(source ${arg_SOURCES})
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
    endforeach()
endfunction()

################################################################################


#TODO: This is probably not the way to do it.
include_directories(
  ${OpenCV_INCLUDE_DIRS}
  ${FREETYPE_INCLUDE_DIRS}
)
add_executable(${PROJECT_NAME}
  src/main.cpp
)

compile_shader(${PROJECT_NAME}
        ENV vulkan
        FORMAT bin
        SOURCES
            src/shaders/simple.vert
            src/shaders/simple.frag
)


target_link_libraries(${PROJECT_NAME}
  ${OpenCV_LIBS}
  ${FREETYPE_LIBRARIES}
        Vulkan::Vulkan glfw glm::glm
        Vulkan::Headers
)
