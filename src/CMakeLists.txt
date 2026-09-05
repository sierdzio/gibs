set(gibs_srcs
    exceptions/commandnotfound.cpp exceptions/commandnotfound.h
    exceptions/commandexception.cpp exceptions/commandexception.h
    exceptions/commanddepthexception.cpp exceptions/commanddepthexception.h
    exceptions/emptylinkobject.cpp exceptions/emptylinkobject.h
    exceptions/targetidtypeexception.cpp exceptions/targetidtypeexception.h
    exceptions/processexception.cpp exceptions/processexception.h
    tools/commandline.cpp tools/commandline.h
    tools/tools.cpp tools/tools.h
    tools/helpdata.cpp tools/helpdata.h
    parsing/syntax.cpp parsing/syntax.h
    parsing/parser.cpp parsing/parser.h
    parsing/paths.cpp parsing/paths.h
    project/project.cpp project/project.h
    project/commandcomponents.cpp project/commandcomponents.h
    project/command.cpp project/command.h
    project/targetid.cpp project/targetid.h
    processing/eventloop.cpp processing/eventloop.cpp
    processing/tool.h
    processing/compiler.cpp processing/compiler.h
    processing/compilerset.cpp processing/compilerset.h
    processing/configuration.cpp processing/configuration.h
    processing/linker.cpp processing/linker.h
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

target_link_libraries(${PROJECT_NAME}-lib PUBLIC
    logger
    process
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
