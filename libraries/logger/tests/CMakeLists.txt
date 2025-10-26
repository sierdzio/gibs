include(CTest)
find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(unit_tests_logger
    test_log.cpp
)

# TODO: investigate, should not be needed
# target_include_directories(unit_tests_logger
#     PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}/../../src/
# )

target_link_libraries(unit_tests_logger
    PRIVATE
    gibs-lib
    GTest::gtest_main
)

gtest_discover_tests(unit_tests_logger)

add_test(NAME unit_tests_logger COMMAND $<TARGET_FILE:unit_tests_logger>)
