#-------------------------------------------------
#
# Project created by QtCreator 2016-10-25T13:50:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET = src
TEMPLATE = app

LIBS += \
    # Путь в рамках shadow build
    -L$$OUT_PWD/../../protocol/lib

SOURCES += main.cpp\
        mainwindow.cpp \
    configdialog.cpp \
    document.cpp \
    terminal_client.cpp

HEADERS  += mainwindow.h \
    configdialog.h \
    document.h \
    terminal_client.h

FORMS    += mainwindow.ui \
    configdialog.ui

# Линкуемся с парсером
include(../../parser/parser.pri)

# Линкуемся с protolink
include(../../protolink/protolink.pri)

# Линкуемся с boost
include(../../boost.pri)

# Линкуемся с библиотекой бинарного протокола
include(../../protocol/protocol.pri)

# Библиотека alpha.protort.components
include(../../components/components.pri)

RESOURCES += \
    images.qrc
