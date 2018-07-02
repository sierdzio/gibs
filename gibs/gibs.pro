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
TARGET = gibs

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
    src/gibs.h \
    src/compiler.h \
    src/deployer.h

SOURCES += src/main.cpp \ 
    src/fileparser.cpp \
    src/projectmanager.cpp \
    src/fileinfo.cpp \
    src/metaprocess.cpp \
    src/baseparser.cpp \
    src/commandparser.cpp \
    src/scope.cpp \
    src/gibs.cpp \
    src/compiler.cpp \
    src/deployer.cpp

RESOURCES +=  \
    qml/qml.qrc \
    resources/resources.qrc

OTHER_FILES += \
    ../gibs.doxyfile \
    ../README.md \
    ../Release.md \
    ../.gitignore \
    ../license-Qt.txt \
    ../.gitlab-ci.yml \
    ../deployment/gibs.desktop \
    ../deployment/gibs.png \
    ../scripts/deploy-linux.sh \
    ../scripts/run-compilation-tests.sh

## Put all build files into build directory
##  This also works with shadow building, so don't worry!
BUILD_DIR = build
OBJECTS_DIR = $$BUILD_DIR
MOC_DIR = $$BUILD_DIR
RCC_DIR = $$BUILD_DIR
UI_DIR = $$BUILD_DIR
DESTDIR = $$BUILD_DIR/out

# Run qmake with CONFIG+=deploy to run the deployment step automatically
deploy {
    message("Copying deployment files. You need to run linuxdeployqt manually.")
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../deployment/gibs.desktop) $$quote($$DESTDIR/) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../deployment/gibs.png) $$quote($$DESTDIR/) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PWD/../scripts/deploy-linux.sh) $$quote($$DESTDIR/) $$escape_expand(\\n\\t)

    # CONFIG+=deploy CONFIG+=auto-deploy
    auto-deploy {
        QMAKE_POST_LINK += $$quote($$DESTDIR/deploy-linux.sh) $$quote($$OUT_PWD/../../linuxdeployqt-continuous-x86_64.AppImage) $$quote($$QMAKE_QMAKE) $$quote($$DESTDIR/gibs.desktop) $$escape_expand(\\n\\t)
        QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$OUT_PWD/gibs-x86_64.AppImage) $$quote($$OUT_PWD/../) $$escape_expand(\\n\\t)
    }
}

## Platforms
include(platforms/mac/mac.pri)
include(platforms/windows/windows.pri)

## Modules
include(../milo/mlog/mlog.pri)
#include(../milo/mconfig/mconfig.pri)
include(../milo/mscripts/mscripts.pri)

DISTFILES += \
    ../LICENSE.txt
