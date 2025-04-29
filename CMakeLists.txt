cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(gibs VERSION 0.0.1
        DESCRIPTION "gibs - generally-in-source build system"
        LANGUAGES CXX)

include(config.cmake)
add_subdirectory(src)

option(BUILD_TESTS "Build gibs test suite" ON) # TODO: make it OFF by default

if (BUILD_TESTS)
    message("Building gibs test suite")
    add_subdirectory(tests)
else()
    message("gibs test suite will not be built")
endif()
