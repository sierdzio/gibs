enable_testing()

find_package(GTest REQUIRED)

add_subdirectory(unit)
add_subdirectory(performance)
