cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(streamer VERSION 0.3.0
        DESCRIPTION "streamer library"
        LANGUAGES CXX)

set(srcs
    streamer/data.cpp
    streamer/data.h
    streamer/json.cpp
    streamer/json.h
    streamer/ini.cpp
    streamer/ini.h
)

add_library(${PROJECT_NAME}
    STATIC
    ${srcs}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<INSTALL_INTERFACE:streamer>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION "${LINK_TIME_OPTIMIZATION}"
    UNITY_BUILD "${UNITY_BUILD_VALUE}"
    UNITY_BUILD_MODE "${UNITY_BUILD_MODE_VALUE}"
    UNITY_BUILD_BATCH_SIZE "${UNITY_BUILD_BATCH_SIZE_VALUE}"
)

option(BUILD_TESTS "Build streamer test suite" OFF)

if (BUILD_TESTS)
    message("Building streamer test suite")
    add_subdirectory(tests)
else()
    message("streamer test suite will not be built")
endif()
