#-----------------------------------------------------------------------

TEMPLATE = app

CONFIG   += release console thread warn_on exceptions stl

QT -= core gui

DEFINES  += TIXML_USE_TICPP
#TIMING_TEST

TARGET = signalserver-sclient

unix:PRE_TARGETDEPS += lib/libssclient.so

win32:PRE_TARGETDEPS += lib/ssclient.lib

DESTDIR = bin
OBJECTS_DIR = tmp

INCLUDEPATH += . include

DEPENDPATH  += $$INCLUDEPATH

INCLUDEPATH += extern/include

#unix: QMAKE_CXXFLAGS += -O3

#-----------------------------------------------------------------------

# Input
SOURCES +=  src/signalserver-client/ssclient_main.cpp


#-----------------------------------------------------------------------

unix {
    LIBS  += -Llib -lssclient \
             -lboost_thread \
             -lboost_system
}

win32 {
    LIBS += lib/ssclient.lib
    # Note: It is assumed that the boost libraries can be automatically detected by the linker
    # through #pragma comment(lib, xxx) declarations in boost.
}

unix {
    # TODO:
    exists( /home/breidi/svn/BCI/HEAD/Common/gdf ) {
#        INCLUDEPATH +=  /home/breidi/svn/BCI/HEAD/Common/gdf

#        LIBS += -L /home/breidi/svn/BCI/HEAD/Common/gdf/libgdf \
#                -lgdf
    }
}

#-----------------------------------------------------------------------
#! end of file

