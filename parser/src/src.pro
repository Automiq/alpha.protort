# Собираем статическую библиотеку
TEMPLATE = lib
CONFIG += staticlib

CONFIG += console c++11

# Qt тут не нужен
CONFIG -= app_bundle
CONFIG -= qt

HEADERS += \
    parser.h

DESTDIR = ../lib

# Общие определения (требуется для LIBINFIX)
include(../../alpha.pri)

# Целевое имя библиотеки
TARGET = alpha.protort.parser$$LIBINFIX

# Линкуемся с boost
include(../../boost.pri)















