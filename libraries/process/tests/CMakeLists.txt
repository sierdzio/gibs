include(CTest)
find_package(GTest REQUIRED)
include(GoogleTest)

set(NAME unit_tests_process)

add_executable(${NAME}
    test_process.cpp
)

target_link_libraries(${NAME}
    PRIVATE
    gibs-lib
    GTest::gtest_main
)

gtest_discover_tests(${NAME})

add_test(NAME ${NAME} COMMAND $<TARGET_FILE:${NAME}>)
