TEMPLATE = lib
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += $$(BOOST_PATH) \
    $$PWD
LIBS += -L$$(BOOST_LIB_PATH)

LIBS += \
    # Путь в рамках shadow build
    -L$$OUT_PWD/../../../protocol/lib

HEADERS += \
    link_client.h \
    link_server.h

