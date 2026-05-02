cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(process VERSION 0.0.1
        DESCRIPTION "process execution library"
        LANGUAGES CXX)

set(srcs
    process/process.cpp process/process.h
    process/stupidprocess.cpp process/stupidprocess.h
    process/systemprocess.h process/systemprocess.cpp
)

add_library(${PROJECT_NAME}
    STATIC
    ${srcs})

target_link_libraries(${PROJECT_NAME} PUBLIC
    logger
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<INSTALL_INTERFACE:process>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION "${LINK_TIME_OPTIMIZATION}"
    UNITY_BUILD "${UNITY_BUILD_VALUE}"
    UNITY_BUILD_MODE "${UNITY_BUILD_MODE_VALUE}"
    UNITY_BUILD_BATCH_SIZE "${UNITY_BUILD_BATCH_SIZE_VALUE}"
)

option(BUILD_TESTS "Build process test suite" OFF)

if (BUILD_TESTS)
    message("Building process test suite")
    add_subdirectory(tests)
else()
    message("process test suite will not be built")
endif()
