cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -Wconversion -Wsign-conversion -pedantic-errors -fPIC")

set(gibs_srcs
    main.cpp
    tools/log.cpp tools/log.h
    tools/commandline.cpp tools/commandline.h
    tools/tools.cpp tools/tools.h
    parsing/syntax.cpp parsing/syntax.h
    parsing/command.cpp parsing/command.h
    parsing/parser.cpp parsing/parser.h
    processing/processor.cpp processing/processor.h
    )

configure_file(versioninfo.h.in versioninfo.h
    NEWLINE_STYLE UNIX
)

# This should probably be moved to a subdir. Should it?
add_executable(gibs ${gibs_srcs} "${CMAKE_CURRENT_BINARY_DIR}/versioninfo.h")

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE OFF
    OUTPUT_NAME "gibs"
    INTERPROCEDURAL_OPTIMIZATION "${LINK_TIME_OPTIMIZATION}"
    UNITY_BUILD "${UNITY_BUILD_VALUE}"
    UNITY_BUILD_MODE "${UNITY_BUILD_MODE_VALUE}"
    UNITY_BUILD_BATCH_SIZE "${UNITY_BUILD_BATCH_SIZE_VALUE}"
)
