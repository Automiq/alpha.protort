TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    main.cpp \    
    test_components.cpp \

LIBS += \
    # Путь в рамках shadow build
    -L$$OUT_PWD/../../../protocol/lib

INCLUDEPATH += $$PWD/../../src

DESTDIR = ../../bin

# Общие определения (требуется для LIBINFIX)
include(../../../alpha.pri)

include(../../../node/node.pri)

TARGET = alpha.protort.components.autotests$$LIBINFIX
