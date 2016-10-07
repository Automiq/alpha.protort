# Собираем статическую библиотеку
TEMPLATE = lib
CONFIG += staticlib

CONFIG += console c++11

# Qt тут не нужен
CONFIG -= app_bundle
CONFIG -= qt

# Общие определения (требуется для LIBINFIX)
include(../../alpha.pri)

LIBS += \
    # Подключаем директорию со сборками protobuf
    -L$$(PROTOBUF_PATH)/lib \
    # Линкуемся с protobuf
    -llibprotobuf$$LIBINFIX

# Файлы описания протокола
PROTOS += \
    protobuf/packet.proto

# Для компиляции протокола с помощью protoc налету
include(protobuf/compiler.pri)
