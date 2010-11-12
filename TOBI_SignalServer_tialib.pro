# -----------------------------------------------------------------------
TEMPLATE = lib
VERSION = 0.1

CONFIG += console \
    release \
    thread \
    warn_on \
    static \
    exceptions \
    stl
QT -= core \
    gui
DEFINES += TIXML_USE_TICPP

# TIMING_TEST
TARGET = tia
DESTDIR = lib
OBJECTS_DIR = tmp
INCLUDEPATH += . \
    include \
    extern/include/LptTools
DEPENDPATH += $$INCLUDEPATH
INCLUDEPATH += extern/include
win32:INCLUDEPATH += extern/include/SDL-1.2.14-VC8

unix{
QMAKE_CXXFLAGS += -pedantic
}

QMAKE_CXXFLAGS_WARN_ON = -Wall -pedantic

# -----------------------------------------------------------------------
HEADERS += include/signalserver/signal_server.h \
    include/definitions/defines.h \
    include/definitions/constants.h \
    include/config/control_message_decoder.h \
    include/config/control_message_encoder.h \
    include/config/control_messages.h \
    include/config/ss_meta_info.h \
    include/datapacket/data_packet.h \
    include/datapacket/raw_mem.h \
    include/network/control_connection.h \
    include/network/control_connection_server.h \
    include/network/tcp_data_server.h \
    include/network/tcp_server.h \
    include/network/udp_data_server.h \
    extern/include/LptTools/LptTools.h
SOURCES += src/signalserver/signal_server.cpp \
    src/definitions/constants.cpp \
    src/config/control_message_decoder.cpp \
    src/config/control_message_encoder.cpp \
    src/config/control_messages.cpp \
    src/config/ss_meta_info.cpp \
    src/datapacket/data_packet.cpp \
    src/datapacket/raw_mem.cpp \
    src/network/control_connection.cpp \
    src/network/control_connection_server.cpp \
    src/network/tcp_data_server.cpp \
    src/network/tcp_server.cpp \
    src/network/udp_data_server.cpp

unix:SOURCES += extern/include/LptTools/LptToolsLinux.cpp
win32:SOURCES += extern/include/LptTools/LptTools_.cpp

# -----------------------------------------------------------------------
unix {
    LIBS += -lboost_thread \
        -lboost_system
    HARDWARE_PLATFORM = $$system(uname -m)
    contains( HARDWARE_PLATFORM, x86_64 )::{
        message(Building 64 bit )
    # 64-bit Linux
    #LIBS += -L \
        #extern/lib/ticpp/linux \
        #-lticpp_64 \
        #-Lextern/lib/libgdf/linux \
        #-llibgdf_64
    }else::{
    # 32-bit Linux
        message(Building 32 bit )

    #LIBS += -Lextern/lib/ticpp/linux \
        #-lticpp \
        #-Lextern/lib/libgdf/linux \
        #-llibgdf
    }
}
win32:LIBS += extern\lib\sdl\win\SDL.lib \
    extern\lib\sdl\win\SDLmain.lib \
    extern\lib\ticpp\win\ticpp.lib \
    extern\lib\g.usbamp\win\gUSBamp.lib \
    extern\lib\libgdf\win\libgdf.lib \
    kernel32.lib \
    advapi32.lib

# Note: It is assumed that the boost libraries can be automatically detected by the linker
# through #pragma comment(lib, xxx) declarations in boost.

