INCLUDEPATH += $$(BOOST_PATH)
LIBS += -L$$(BOOST_LIB_PATH)

linux {
    LIBS += -lboost_unit_test_framework
    DEFINES *= BOOST_TEST_DYN_LINK
}
