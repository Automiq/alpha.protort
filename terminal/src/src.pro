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


SOURCES += main.cpp\
        mainwindow.cpp \
    configdialog.cpp \
    document.cpp

HEADERS  += mainwindow.h \
    configdialog.h \
    document.h

FORMS    += mainwindow.ui \
    configdialog.ui

# Линкуемся с парсером
include(../../parser/parser.pri)

RESOURCES += \
    images.qrc
