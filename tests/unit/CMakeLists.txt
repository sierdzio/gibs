cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -Wconversion -Wsign-conversion -pedantic-errors -fPIC")

enable_testing()

find_package(GTest REQUIRED)

add_executable(unit_tests
    test_tools.cpp
)

# TODO: investigate, should not be needed
target_include_directories(unit_tests
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/
)

target_link_libraries(unit_tests
    PRIVATE
    gibs-lib
    gtest
    gtest_main
)

gtest_discover_tests(unit_tests)
