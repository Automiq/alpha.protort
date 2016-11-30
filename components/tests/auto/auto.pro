TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    main.cpp \    
    test_components.cpp \

INCLUDEPATH += $$PWD/../../src

DESTDIR = ../../bin

# Общие определения (требуется для LIBINFIX)
include(../../../alpha.pri)

TARGET = alpha.protort.components.autotests$$LIBINFIX

# Линкуемся с boost
include(../../../boost.pri)

# Линкуемся с components
include(../../components.pri)
