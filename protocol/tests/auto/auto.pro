TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    main.cpp \
    tests_packet.cpp

DESTDIR = ../../bin

TARGET = alpha.protort.protocol.autotests

LIBS += \
    # Путь в рамках shadow build
    -L$$OUT_PWD/../../lib

# Линкуемся с boost
include(../../../boost.pri)

# Линкуемся с библиотекой бинарного протокола
include(../../../protocol/protocol.pri)
