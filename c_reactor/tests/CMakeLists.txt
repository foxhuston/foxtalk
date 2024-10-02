set(CMAKE_CXX_STANDARD 26)

set (Boost_USE_STATIC_LIBS OFF)
find_package (Boost REQUIRED COMPONENTS unit_test_framework)
include_directories (${Boost_INCLUDE_DIRS})

add_executable (Boost_Tests_run DbTests.cpp)

target_include_directories(Boost_Tests_run PRIVATE ../lib)
target_include_directories(Boost_Tests_run PRIVATE ../vendor/gc-8.2.4/include)

target_link_libraries (Boost_Tests_run ${Boost_LIBRARIES})
target_link_libraries (Boost_Tests_run reactor gc)
