INCLUDEPATH += $$PWD/src

HEADERS += \
    $$PWD/src/server.h \
    $$PWD/src/client.h \
    $$PWD/src/connection.h \
    $$PWD/src/packet_header.h \
    $$PWD/src/protoserver.h \
    $$PWD/src/protoconnection.h \
    $$PWD/src/protoclient.h

# Линкуемся с boost
include(../../../boost.pri)
