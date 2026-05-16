include(GoogleTest)

add_executable(unit_tests_gibs
    tools/test_tools.cpp
    tools/test_commandline.cpp
    parsing/test_syntax.cpp
    parsing/test_parser.cpp
    project/test_targetid.cpp
    project/test_command.cpp
    project/test_project.cpp
    processing/test_tool.cpp
)

# TODO: investigate, should not be needed
target_include_directories(unit_tests_gibs
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/
)

target_link_libraries(unit_tests_gibs
    PRIVATE
    gibs-lib
    GTest::gtest_main
)

gtest_discover_tests(unit_tests_gibs)

add_test(NAME unit_tests_gibs COMMAND $<TARGET_FILE:unit_tests_gibs>)
