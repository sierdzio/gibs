project(subproject VERSION 0.0.1
        DESCRIPTION "An example project for gibs"
        LANGUAGES CXX)

add_subdirectory(app)
add_subdirectory(library)