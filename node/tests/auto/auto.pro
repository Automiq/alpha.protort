TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    main.cpp \
    tests_node.cpp \
    tests_node_settings.cpp \
    tests_node_router.cpp \
    tests_node_deploy.cpp

INCLUDEPATH += $$PWD/../../src

DESTDIR = ../../bin

# Общие определения (требуется для LIBINFIX)
include(../../../alpha.pri)

TARGET = alpha.protort.node.autotests$$LIBINFIX

LIBS += \
    # Путь в рамках shadow build
    -L$$OUT_PWD/../../../protocol/lib

# Линкуемся с boost
include(../../../boost.pri)

# Подключаем библиотеку link
include(../../../link/link.pri)

# Линкуемся с библиотекой бинарного протокола
include(../../../protocol/protocol.pri)

# Библиотека alpha.protort.components
include(../../../components/components.pri)

# Линкуемся с парсером xml
include(../../parser.pri)

# Копируем файлы в зависимости от наличия shadow build
!equals(PWD, $${OUT_PWD}) {
    win32 {
        copyapp.commands = $(COPY_DIR) \"$$PWD\testdata\\*\" \"$$OUT_PWD\testdata\"
    } else {
        copydata.commands = $(COPY_DIR) $$PWD/testdata/* $$OUT_PWD/tesdata
    }
}
first.depends = $(first) copyapp
export(first.depends)
export(copyapp.commands)
QMAKE_EXTRA_TARGETS += first copyapp
