INCLUDEPATH += $$(BOOST_PATH)
LIBS += -L$$(BOOST_LIB_PATH)

linux {
    LIBS += -lboost_unit_test_framework
}
