## Milo Solutions - project file TEMPLATE
#
#
## (c) Milo Solutions, 2016

QT = core

# Warning! QStringBuilder can crash your app! See last point here:
# https://www.kdab.com/uncovering-32-qt-best-practices-compile-time-clazy/
# !!!
DEFINES *= QT_USE_QSTRINGBUILDER
QMAKE_CXXFLAGS += -Werror

TEMPLATE = app
CONFIG += c++14
TARGET = ibs

HEADERS += src/globals.h \
    src/fileparser.h \
    src/projectmanager.h \
    src/tags.h \
    src/flags.h \
    src/fileinfo.h \
    src/metaprocess.h \
    src/baseparser.h \
    src/commandparser.h \
    src/scope.h \
    src/ibs.h

SOURCES += src/main.cpp \ 
    src/fileparser.cpp \
    src/projectmanager.cpp \
    src/fileinfo.cpp \
    src/metaprocess.cpp \
    src/baseparser.cpp \
    src/commandparser.cpp \
    src/scope.cpp \
    src/ibs.cpp

RESOURCES +=  \
    qml/qml.qrc \
    resources/resources.qrc

OTHER_FILES += \
    ../ibs.doxyfile \
    ../README.md \
    ../Release.md \
    ../.gitignore \
    ../license-Qt.txt \
    ../.gitlab-ci.yml

## Put all build files into build directory
##  This also works with shadow building, so don't worry!
BUILD_DIR = build
OBJECTS_DIR = $$BUILD_DIR
MOC_DIR = $$BUILD_DIR
RCC_DIR = $$BUILD_DIR
UI_DIR = $$BUILD_DIR
DESTDIR = $$BUILD_DIR/bin

## Platforms
include(platforms/mac/mac.pri)
include(platforms/windows/windows.pri)

## Modules
include(../milo/mlog/mlog.pri)
include(../milo/mconfig/mconfig.pri)
include(../milo/mscripts/mscripts.pri)
