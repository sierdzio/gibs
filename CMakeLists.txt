cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(gibs VERSION 0.2.0
        DESCRIPTION "gibs - generally-in-source build system"
        LANGUAGES CXX)

option(SYNCHRONISE_LOG_MESSAGES "Use mutex to make sure log messages are not getting mixed between threads" OFF)
option(BUILD_TESTS "Build gibs test suite" OFF)
option(BUILD_SAMPLES "Build gibs samples" OFF)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(global.cmake)
include(config.cmake)

add_subdirectory(libraries/logger)
add_subdirectory(libraries/process)
add_subdirectory(src)

if (BUILD_TESTS)
    message("Building gibs test suite")
    option(BUILD_SAMPLE_TESTS "Run gibs on all samples" OFF)
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
