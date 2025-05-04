set(gibs_srcs
    exceptions/commandnotfound.cpp exceptions/commandnotfound.h
    exceptions/commanddepthexception.cpp exceptions/commanddepthexception.h
    exceptions/emptylinkobject.cpp exceptions/emptylinkobject.h
    exceptions/loglevelexception.cpp exceptions/loglevelexception.h
    exceptions/targetidtypeexception.cpp exceptions/targetidtypeexception.h
    tools/log.cpp tools/log.h
    tools/commandline.cpp tools/commandline.h
    tools/tools.cpp tools/tools.h
    parsing/syntax.cpp parsing/syntax.h
    parsing/parser.cpp parsing/parser.h
    project/project.cpp project/project.h
    project/commandcomponents.cpp project/commandcomponents.h
    project/command.cpp project/command.h
    project/targetid.cpp project/targetid.h
    processing/processor.cpp processing/processor.h
    )

configure_file(versioninfo.h.in versioninfo.h
    NEWLINE_STYLE UNIX
)

add_library(${PROJECT_NAME}-lib
    STATIC
    ${gibs_srcs}
    "${CMAKE_CURRENT_BINARY_DIR}/versioninfo.h"
)

set_target_properties(${PROJECT_NAME}-lib PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION "${LINK_TIME_OPTIMIZATION}"
    UNITY_BUILD "${UNITY_BUILD_VALUE}"
    UNITY_BUILD_MODE "${UNITY_BUILD_MODE_VALUE}"
    UNITY_BUILD_BATCH_SIZE "${UNITY_BUILD_BATCH_SIZE_VALUE}"
)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}-lib)

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE OFF
    OUTPUT_NAME ${PROJECT_NAME}
    INTERPROCEDURAL_OPTIMIZATION "${LINK_TIME_OPTIMIZATION}"
    UNITY_BUILD "${UNITY_BUILD_VALUE}"
    UNITY_BUILD_MODE "${UNITY_BUILD_MODE_VALUE}"
    UNITY_BUILD_BATCH_SIZE "${UNITY_BUILD_BATCH_SIZE_VALUE}"
)
