TEMPLATE += app
CONFIG += console
TARGET = $$PWD/bin/test

QT -= core \
      gui


INCLUDEPATH += $$PWD/. \
    $$PWD/include \
    $$PWD/extern/include
DEPENDPATH += $$INCLUDEPATH

DEFINES += TIXML_USE_TICPP

#QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
#QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage
#LIBS += -lgcov

LIBS += $$PWD/lib/libtia.a

HARDWARE_PLATFORM = $$system(uname -m)

contains( HARDWARE_PLATFORM, x86_64 )::{
  LIBS += $$PWD/extern/lib/ticpp/linux/libticpp_64.a
}
else:: {
  LIBS += $$PWD/extern/lib/ticpp/linux/libticpp.a
}

LIBS += -lboost_thread \
        -lboost_system \
        -lboost_filesystem \
        -L$$PWD/extern/lib \
        -L$$PWD/extern/lib/ticpp/linux \
        -L$$PWD/tests/UnitTest++ \
        -lUnitTest++ \
        -lticpp

SOURCES += \
    tests/main.cpp \
    tests/datapacket_tests.cpp \
    tests/tia_client_tests.cpp \
    tests/tia_clock_tests.cpp \
    tests/tia_server_tests.cpp \
    tests/server_control_connection_tests.cpp

HEADERS += \
    tests/datapacket_tests_fixtures.h \
    tests/raw_datapacket_definition.h \
    tests/tia_server_control_connection_tests_fixtures.h \
    tests/tia_control_messages_definition.h
