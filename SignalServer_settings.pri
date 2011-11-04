# -----------------------------------------------------------------------
TEMPLATE += app
CONFIG += console \
    release \
    thread \
    warn_on \
    exceptions \
    stl
QT -= core \
    gui

DEFINES += TIXML_USE_TICPP
#DEFINES += USE_TID_SERVER
#DEFINES += USE_GDF_SAVER
#DEFINES += TIMING_TEST

TARGET = signalserver

DESTDIR = bin
OBJECTS_DIR = tmp
INCLUDEPATH += . \
    include \
    extern/include/LptTools

DEPENDPATH += $$INCLUDEPATH
INCLUDEPATH += extern/include\
               extern/include/TiDlib

win32:INCLUDEPATH += extern/include/SDL-1.2.14-VC8

unix {
    QMAKE_CXXFLAGS += -pedantic
    versiontarget.target = ./update-version.sh
    versiontarget.commands = ./update-version.sh
    versiontarget.depends = FORCE
    PRE_TARGETDEPS += update-version.sh
    QMAKE_EXTRA_TARGETS += versiontarget
}

QMAKE_CXXFLAGS_WARN_ON = -Wall \
    -pedantic

# ------------------------------------------

HARDWARE_PLATFORM = $$system(uname -m)
contains( HARDWARE_PLATFORM, x86_64 )::
  {
    message(Building 64 bit )
  }else::{
    message(Building 32 bit )
  }

# ------------------------------------------
