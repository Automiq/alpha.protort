TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

DESTDIR = ../bin

# Общие определения (требуется для LIBINFIX)
include(../../alpha.pri)

TARGET = alpha.protort.node$$LIBINFIX

LIBS += \
    # Путь в рамках shadow build
    -L$$OUT_PWD/../../protocol/lib

include(../node.pri)

HEADERS += \
    ioswrapper.h
