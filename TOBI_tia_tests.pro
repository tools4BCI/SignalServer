TEMPLATE += app
CONFIG += console
TARGET = $$PWD/bin/test

QT -= core \
      gui


INCLUDEPATH += $$PWD/include

LIBS += $$PWD/lib/libtia.a

LIBS += $$PWD/extern/lib/ticpp/linux/libticpp.a

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
