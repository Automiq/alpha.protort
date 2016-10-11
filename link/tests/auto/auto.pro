TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    main.cpp \
    tests_link.cpp

DESTDIR = ../bin

TARGET = alpha.protort.link.autotests

LIBS += \
    # Путь в рамках shadow build
    -L$$OUT_PWD/../../../protocol/lib

# Линкуемся с boost
include(../../../boost.pri)

# Линкуемся с библиотекой бинарного протокола
include(../../../protocol/protocol.pri)
