TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    main.cpp \
    tests_parser.cpp

INCLUDEPATH += $$PWD/../../src

DESTDIR = ../../bin

# Общие определения (требуется для LIBINFIX)
include(../../../alpha.pri)

TARGET = alpha.protort.parser.autotests$$LIBINFIX

# Линкуемся с boost
include(../../../boost.pri)
