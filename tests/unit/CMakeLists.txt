include(GoogleTest)

add_executable(unit_tests
    tools/test_tools.cpp
    tools/test_log.cpp
    tools/test_commandline.cpp
    parsing/test_syntax.cpp
    project/test_targetid.cpp
    project/test_command.cpp
)

# TODO: investigate, should not be needed
target_include_directories(unit_tests
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/
)

target_link_libraries(unit_tests
    PRIVATE
    gibs-lib
    GTest::gtest_main
)

gtest_discover_tests(unit_tests)

add_test(NAME unit_tests COMMAND $<TARGET_FILE:unit_tests>)
