cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(multiple-files-and-libs-lib VERSION 0.0.1
        DESCRIPTION "An example lib for gibs"
        LANGUAGES CXX)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(${PROJECT_NAME} STATIC exported.cpp libraryclass.cpp)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<INSTALL_INTERFACE:${PROJECT_NAME}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
)
