include(CTest)
find_package(GTest REQUIRED)
include(GoogleTest)

set(NAME unit_tests_logger)

add_executable(${NAME}
    test_log.cpp
)

target_link_libraries(${NAME}
    PRIVATE
    gibs-lib
    GTest::gtest_main
)

if (GTEST_DISCOVERY_EXECUTOR)
    set_property(TARGET ${NAME} PROPERTY TEST_LAUNCHER "${GTEST_DISCOVERY_EXECUTOR}")
endif ()

gtest_discover_tests(${NAME})
