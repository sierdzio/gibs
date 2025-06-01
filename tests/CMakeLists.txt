project(tests VERSION 0.0.1
        DESCRIPTION "gibs - tests"
        LANGUAGES CXX)

find_package(GTest REQUIRED)

add_subdirectory(unit)
add_subdirectory(performance)
