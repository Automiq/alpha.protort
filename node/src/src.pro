TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp
HEADERS += \
    node_settings.h

DESTDIR = ../bin

# Общие определения (требуется для LIBINFIX)
include(../../alpha.pri)

TARGET = alpha.protort.node$$LIBINFIX

LIBS += \
    # Путь в рамках shadow build
    -L$$OUT_PWD/../../protocol/lib

# Линкуемся с boost
include(../../boost.pri)

# Линкуемся с библиотекой бинарного протокола
include(../../protocol/protocol.pri)
