INCLUDEPATH += $$PWD/src

HEADERS += \
    $$PWD/src/node_settings.h \
    $$PWD/src/node.h \
    $$PWD/src/router.h

# Линкуемся с boost
include(../boost.pri)

# Подключаем библиотеку protolink
include(../protolink/protolink.pri)

# Линкуемся с библиотекой бинарного протокола
include(../protocol/protocol.pri)

# Библиотека alpha.protort.components
include(../components/components.pri)

# Линкуемся с парсером xml
include(../parser/parser.pri)
