QT = core

TARGET = simpletest
TEMPLATE = app

INCLUDEPATH = ../library

LIBS = -L$$OUT_PWD/../library -lsimpletest

SOURCES = main.cpp
