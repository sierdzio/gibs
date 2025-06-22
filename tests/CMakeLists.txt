project(test VERSION 0.0.1
        DESCRIPTION "gibs - tests"
        LANGUAGES CXX)

include(CTest)
find_package(GTest REQUIRED)

add_subdirectory(unit)
add_subdirectory(performance)
