# -----------------------------------------------------------------------
TEMPLATE = app
CONFIG += release \
    console \
    thread \
    warn_on \
    exceptions \
    stl
QT -= core \
    gui
DEFINES += TIXML_USE_TICPP

# TIMING_TEST
TARGET = signalserver-sclient
unix:PRE_TARGETDEPS += $$PWD/lib/libtia.so
win32:PRE_TARGETDEPS += $$PWD/lib/tia.lib
DESTDIR = $$PWD/bin
OBJECTS_DIR = tmp
INCLUDEPATH += . \
    include
DEPENDPATH += $$INCLUDEPATH
INCLUDEPATH += extern/include

QMAKE_CXXFLAGS_WARN_ON = -Wall -pedantic

# unix: QMAKE_CXXFLAGS += -O3
# -----------------------------------------------------------------------
# Input
SOURCES += src/ssclient_main.cpp

# -----------------------------------------------------------------------
unix:LIBS += -L$$PWD/lib -L$$PWD/extern/lib/ticpp/linux \
    -ltia -lboost_thread
win32:LIBS += lib/tia.lib

# Note: It is assumed that the boost libraries can be automatically detected by the linker
# through #pragma comment(lib, xxx) declarations in boost.

