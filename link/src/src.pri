INCLUDEPATH += $$(BOOST_PATH) \
    $$PWD
LIBS += -L$$(BOOST_LIB_PATH)


LIBS += \
    # Путь в рамках shadow build
    -L$$OUT_PWD/../../../protocol/lib

HEADERS += \
    server.h \
    client.h \
    connection.h \
    packet_header.h


