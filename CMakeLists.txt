cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(gibs VERSION 0.0.1
        DESCRIPTION "gibs - grand-in-source build system"
        LANGUAGES CXX)

include(config.cmake)

add_subdirectory(src)

set(other_files
    .gitignore
    .travis.yml
    AUTHORS.md
    gibs.doxyfile
    LICENSE.txt
    README.md
    )

set_source_files_properties(${other_files} PROPERTIES HEADER_FILE_ONLY TRUE)
add_custom_target(other_files SOURCES ${other_files})
