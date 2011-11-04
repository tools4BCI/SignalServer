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

TARGET = TiA_client

DESTDIR = bin
OBJECTS_DIR = tmp
INCLUDEPATH += . \
    include

DEPENDPATH += $$INCLUDEPATH
INCLUDEPATH += extern/include

QMAKE_CXXFLAGS_WARN_ON = -Wall -pedantic
# unix: QMAKE_CXXFLAGS += -O3

# -----------------------------------------------------------------------

SOURCES += src/tia_client_main.cpp

# -----------------------------------------------------------------------
unix {
    LIBS +=  -Lextern/lib/ticpp/linux \
     -lboost_thread -lboost_system


    HARDWARE_PLATFORM = $$system(uname -m)
    contains( HARDWARE_PLATFORM, x86_64 )::
    {
        # 64-bit Linux
        LIBS += -lticpp_64 \
                -L extern/lib/tia/linux/x64 -l tia

    }else::{

        # 32-bit Linux
        LIBS += -lticpp \
                -L extern/lib/tia/linux/x64 -l tia
    }
}

win32:LIBS += lib/tia.lib

# Note: It is assumed that the boost libraries can be automatically detected by the linker
# through #pragma comment(lib, xxx) declarations in boost.
