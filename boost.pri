INCLUDEPATH += $$(BOOST_PATH)
LIBS += -L$$(BOOST_LIB_PATH)

linux {
    LIBS += -lboost_system -lboost_program_options -lboost_unit_test_framework -lboost_chrono -lpthread -lboost_thread
    DEFINES *= BOOST_TEST_DYN_LINK
}
