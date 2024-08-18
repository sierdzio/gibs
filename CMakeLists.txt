cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(gibs VERSION 0.0.1
        DESCRIPTION "gibs - generally-in-source build system"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -fPIC")

set(gibs_srcs
    src/main.cpp
    src/log.cpp
    )

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

# This should probably be moved to a subdir. Should it?
add_executable(gibs ${gibs_srcs})

#target_compile_features(gibs PUBLIC cxx_std_14)
