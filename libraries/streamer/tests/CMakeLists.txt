include(CTest)
find_package(GTest REQUIRED)
include(GoogleTest)

set(NAME unit_tests_streamer)

add_executable(${NAME}
    test_json.cpp
    test_ini.cpp
)

target_link_libraries(${NAME}
    PRIVATE
    streamer
    GTest::gtest_main
)

if (GTEST_DISCOVERY_EXECUTOR)
    set_property(TARGET ${NAME} PROPERTY TEST_LAUNCHER "${GTEST_DISCOVERY_EXECUTOR}")
endif ()

gtest_discover_tests(${NAME})
