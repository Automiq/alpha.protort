message("Linking to alpha.protort.protocol library")

include(../alpha.pri)

INCLUDEPATH += $$(PROTOBUF_PATH)/include \
    $$PWD/src/protobuf

LIBS += \
    # Путь к папке с библиотекой protobuf
    -L$$(PROTOBUF_PATH)/lib \
    # Путь к папке с библиотекой в рамках обычного билда
    -L$$PWD/lib \
    # Линкуем protobuf
    -l$${LIBPREFIX}protobuf$$LIBINFIX \
    # Линкуем библиотеку бинарного протокола
    -lalpha.protort.protocol$$LIBINFIX
