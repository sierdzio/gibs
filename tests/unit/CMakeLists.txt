add_executable(unit_tests
    project/test_targetid.cpp
    tools/test_tools.cpp
    tools/test_log.cpp
    tools/test_commandline.cpp
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
