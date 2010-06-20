#-----------------------------------------------------------------------

TEMPLATE = app

CONFIG   += release console thread warn_on static exceptions stl

DEFINES  += TIXML_USE_TICPP
#TIMING_TEST

TARGET = signalserver-sclient

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
    LIBS += -L lib ssclient.dll
                
    # Note: It is assumed that the boost libraries can be automatically detected by the linker 
    # through #pragma comment(lib, xxx) declarations in boost.
}

unix {
    # TODO: 
    exists( /home/breidi/svn/BCI/HEAD/Common/gdf ) {
        INCLUDEPATH +=  /home/breidi/svn/BCI/HEAD/Common/gdf
    
        LIBS += -L /home/breidi/svn/BCI/HEAD/Common/gdf/libgdf \
                -lgdf
    }
}

include ( TOBI_SignalServer_client_customize.pro )

#-----------------------------------------------------------------------
#! end of file

