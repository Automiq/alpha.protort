#-------------------------------------------------
#
# Project created by QtCreator 2016-10-25T13:50:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = src
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

# Линкуемся с парсером
include(../../parser/parser.pri)

DISTFILES += \
    images/editcopy.png \
    images/editcut.png \
    images/editpaste.png \
    images/filenew.png \
    images/fileopen.png \
    images/filesave.png \
    images/fileSaveAll.png \
    images/parse.png \
    images/deploy.png \
    images/stop.png
