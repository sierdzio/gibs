add_executable(unit_tests
    test_tools.cpp
    test_log.cpp
)

# TODO: investigate, should not be needed
target_include_directories(unit_tests
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/
)

target_link_libraries(unit_tests
    PRIVATE
    gibs-lib
    gtest
    gtest_main
)

gtest_discover_tests(unit_tests)
