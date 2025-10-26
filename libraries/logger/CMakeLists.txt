cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(logger VERSION 0.0.1
        DESCRIPTION "logging library"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -Wconversion -Wsign-conversion -pedantic-errors -fPIC")

set(srcs
    logger/exceptions/loglevelexception.cpp logger/exceptions/loglevelexception.h
    logger/log.cpp logger/log.h
)

add_library(${PROJECT_NAME}
    STATIC
    ${srcs}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<INSTALL_INTERFACE:logger>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION "${LINK_TIME_OPTIMIZATION}"
    UNITY_BUILD "${UNITY_BUILD_VALUE}"
    UNITY_BUILD_MODE "${UNITY_BUILD_MODE_VALUE}"
    UNITY_BUILD_BATCH_SIZE "${UNITY_BUILD_BATCH_SIZE_VALUE}"
)

option(BUILD_TESTS "Build logger test suite" ON) # TODO: make it OFF by default

if (BUILD_TESTS)
    message("Building logger test suite")
    add_subdirectory(tests)
else()
    message("logger test suite will not be built")
endif()
