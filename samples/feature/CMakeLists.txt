cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(feature VERSION 0.0.1
        DESCRIPTION "An example project for gibs"
        LANGUAGES CXX)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(${PROJECT_NAME} main.cpp someclass.cpp)
