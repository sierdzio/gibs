cmake_minimum_required(VERSION 3.9...3.12)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(gibs VERSION 0.0.1
        DESCRIPTION "gibs - in-source build system"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

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

# This should probably be moved to a subdir. Should it?
add_executable(gibs ${gibs_srcs} ${other_files})

#target_compile_features(gibs PUBLIC cxx_std_14)
