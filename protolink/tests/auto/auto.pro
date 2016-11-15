TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    main.cpp

DESTDIR = ../../bin

# Общие определения (требуется для LIBINFIX)
include(../../../alpha.pri)

TARGET = alpha.protort.link.autotests$$LIBINFIX

LIBS += \
    # Путь в рамках shadow build
    -L$$OUT_PWD/../../../protocol/lib

# Подключим нужные хедеры
INCLUDEPATH += $$PWD/../../src

# Линкуемся с boost
include(../../../boost.pri)

include(../../link.pri)

# Линкуемся с библиотекой бинарного протокола
include(../../../protocol/protocol.pri)
