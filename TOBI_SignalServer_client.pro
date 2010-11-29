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
unix:PRE_TARGETDEPS += lib/libtiaclient.so
win32:PRE_TARGETDEPS += lib/ssclient.lib
DESTDIR = bin
OBJECTS_DIR = tmp
INCLUDEPATH += . \
    include
DEPENDPATH += $$INCLUDEPATH
INCLUDEPATH += extern/include

QMAKE_CXXFLAGS_WARN_ON = -Wall -pedantic

# unix: QMAKE_CXXFLAGS += -O3
# -----------------------------------------------------------------------
# Input
SOURCES += src/signalserver-client/ssclient_main.cpp

# -----------------------------------------------------------------------
unix:LIBS += -Llib -Lextern/lib/ticpp/linux \
    -ltiaclient -lboost_thread
win32:LIBS += lib/ssclient.lib

# Note: It is assumed that the boost libraries can be automatically detected by the linker
# through #pragma comment(lib, xxx) declarations in boost.

