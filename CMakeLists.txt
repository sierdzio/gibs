cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(gibs VERSION 0.0.1
        DESCRIPTION "gibs - generally-in-source build system"
        LANGUAGES CXX)

option(SYNCHRONISE_LOG_MESSAGES "Use mutex to make sure log messages are not getting mixed between threads" OFF)
option(BUILD_TESTS "Build gibs test suite" OFF)
option(BUILD_SAMPLES "Build gibs samples" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -Wconversion -Wsign-conversion -pedantic-errors -fPIC")

include(config.cmake)
add_subdirectory(libraries/logger)
add_subdirectory(libraries/process)
add_subdirectory(src)

if (BUILD_TESTS)
    message("Building gibs test suite")
    enable_testing()
    add_subdirectory(tests)
else()
    message("gibs test suite will not be built")
endif()

if (BUILD_SAMPLES)
    message("Building gibs samples")
    add_subdirectory(samples)
else()
    message("gibs samples will not be built")
endif()
