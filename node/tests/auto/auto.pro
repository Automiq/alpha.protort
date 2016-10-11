TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    main.cpp \
    tests_node.cpp

DESTDIR = ../bin

TARGET = alpha.protort.node.autotests

LIBS += \
    # Путь в рамках shadow build
    -L$$OUT_PWD/../../../protocol/lib

# Линкуемся с boost
include(../../../boost.pri)

# Линкуемся с библиотекой бинарного протокола
include(../../../protocol/protocol.pri)
