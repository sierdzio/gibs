cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(gibs VERSION 0.0.1
        DESCRIPTION "gibs - generally-in-source build system"
        LANGUAGES CXX)

include(config.cmake)
add_subdirectory(src)
