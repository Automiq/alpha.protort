TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    main.cpp \
    tests_parser.cpp

INCLUDEPATH += $$PWD/../../src

DESTDIR = ../../bin

# Общие определения (требуется для LIBINFIX)
include(../../../alpha.pri)

TARGET = alpha.protort.parser.autotests$$LIBINFIX

# Линкуемся с boost
include(../../../boost.pri)

win32 {
    copydata.commands = $(COPY_DIR) \"$$PWD/xmls\" \"$$OUT_PWD/../../bin\"
} else {
    copydata.commands = $(COPY_DIR) $$PWD/xmls $$OUT_PWD/../../bin
}

first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata


