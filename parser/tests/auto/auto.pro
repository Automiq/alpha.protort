TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    main.cpp \
    tests_parser.cpp

DESTDIR = ./bin

INCLUDEPATH += $$PWD/../../src

# Общие определения (требуется для LIBINFIX)
include(../../../alpha.pri)

TARGET = alpha.protort.parser.autotests$$LIBINFIX

# Линкуемся с boost
include(../../../boost.pri)

# Линкуемся с парсером xml
include(../../parser.pri)

# Копируем файлы в зависимости от наличия shadow build
!equals(PWD, $${OUT_PWD}) {
    win32 {
        copyapp.commands = $(COPY_DIR) \"$$PWD\testdata\\*\" \"$$OUT_PWD\testdata\"
    } else {
        copydata.commands = $(COPY_DIR) $$PWD/testdata/* $$OUT_PWD/testdata
    }
}
first.depends = $(first) copyapp
export(first.depends)
export(copyapp.commands)
QMAKE_EXTRA_TARGETS += first copyapp
