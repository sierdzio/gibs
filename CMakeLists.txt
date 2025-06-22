cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(gibs VERSION 0.0.1
        DESCRIPTION "gibs - generally-in-source build system"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -Wconversion -Wsign-conversion -pedantic-errors -fPIC")

include(config.cmake)
add_subdirectory(src)
add_subdirectory(libraries/process)

option(BUILD_TESTS "Build gibs test suite" ON) # TODO: make it OFF by default

if (BUILD_TESTS)
    message("Building gibs test suite")
    enable_testing()
    add_subdirectory(tests)
else()
    message("gibs test suite will not be built")
endif()
