INCLUDEPATH += $$PWD/src

HEADERS += \
    $$PWD/../link/src/packet_header.h \
    $$PWD/src/protoserver.h \
    $$PWD/src/protoconnection.h \
    $$PWD/src/protoclient.h

# Линкуемся с boost
include(../../../boost.pri)
