TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

HEADERS += \
    i_component.h \
    retranslator.h \    
    generator.h \
    terminator.h

# Линкуемся с boost
include(../../boost.pri)

# Подключаем библиотеку link
include(../../link/link.pri)

