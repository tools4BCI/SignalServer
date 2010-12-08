TEMPLATE += app
CONFIG += console
TARGET = $$PWD/bin/test

QT -= core \
      gui


INCLUDEPATH += $$PWD/. \
    $$PWD/include
DEPENDPATH += $$INCLUDEPATH

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
        -L$$PWD/tests/UnitTest++ \
        -lUnitTest++

SOURCES += \
    tests/main.cpp \
    tests/datapacket_tests.cpp \
    tests/tia_client_tests.cpp

HEADERS += \
    tests/datapacket_tests_fixtures.h \
    tests/raw_datapacket_definition.h
