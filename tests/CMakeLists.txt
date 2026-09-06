project(test VERSION 0.0.1
        DESCRIPTION "gibs - tests"
        LANGUAGES CXX)

include(CTest)
find_package(GTest REQUIRED)

add_subdirectory(unit)
add_subdirectory(performance)

if (BUILD_SAMPLE_TESTS)
        message("Building gibs sample tests")
        add_subdirectory(samples)
endif()
