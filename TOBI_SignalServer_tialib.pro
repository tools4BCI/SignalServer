# -----------------------------------------------------------------------
# ! end of file
# -----------------------------------------------------------------------
# ! end of file
# -----------------------------------------------------------------------
# ! end of file
# -----------------------------------------------------------------------
# ! end of file
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
DESTDIR = $$PWD/lib
OBJECTS_DIR = tmp
INCLUDEPATH += $$PWD/. \
    $$PWD/include \
    $$PWD/extern/include/LptTools
DEPENDPATH += $$INCLUDEPATH
INCLUDEPATH += $$PWD/extern/include
win32:INCLUDEPATH += $$PWD/extern/include/SDL-1.2.14-VC8
unix:QMAKE_CXXFLAGS += -pedantic
QMAKE_CXXFLAGS_WARN_ON = -Wall \
    -pedantic

# -----------------------------------------------------------------------
HEADERS += include/tia/tia_server.h \
    include/tia/defines.h \
    include/tia/constants.h \
    include/tia-private/config/control_message_decoder.h \
    include/tia-private/config/control_message_encoder.h \
    include/tia-private/config/control_messages.h \
    include/tia/ss_meta_info.h \
    include/tia/data_packet.h \
    include/tia-private/datapacket/raw_mem.h \
    include/tia-private/datapacket/data_packet_selector.h \
    include/tia-private/network/control_connection.h \
    include/tia-private/network/control_connection_server.h \
    include/tia-private/network/tcp_data_server.h \
    include/tia-private/network/tcp_server.h \
    include/tia-private/network/udp_data_server.h \
    include/tia/tia_client.h \
    include/tia-private/client/tia_client_impl.h \
    include/tia-private/client/tia_client_impl_base.h \
    include/tia/ssconfig.h \
    extern/include/LptTools/LptTools.h \
    include/tia/data_packet3.h \
    include/tia-private/datapacket/raw_mem3.h \
    include/tia-private/clock.h \
    include/tia-private/server/control_connection_server_2.h \
    include/tia-private/server/server_command.h \
    include/tia-private/server/socket.h \
    include/tia-private/server/stop_data_transmission_server_command.h \
    include/tia-private/server/data_server.h \
    include/tia-private/server/start_data_transmission_server_command.h
SOURCES += src/tia/tia_server.cpp \
    src/tia/constants.cpp \
    src/tia/config/control_message_decoder.cpp \
    src/tia/config/control_message_encoder.cpp \
    src/tia/config/control_messages.cpp \
    src/tia/ss_meta_info.cpp \
    src/tia/data_packet.cpp \
    src/tia/datapacket/raw_mem.cpp \
    src/tia/datapacket/data_packet_selector.cpp \
    src/tia/network/control_connection.cpp \
    src/tia/network/control_connection_server.cpp \
    src/tia/network/tcp_data_server.cpp \
    src/tia/network/tcp_server.cpp \
    src/tia/network/udp_data_server.cpp \
    src/tia/client/tia_client_impl.cpp \
    src/tia/tia_client.cpp \
    src/tia/data_packet3.cpp \
    src/tia/datapacket/raw_mem3.cpp \
    src/tia/clock.cpp
unix:SOURCES += extern/include/LptTools/LptToolsLinux.cpp
win32:SOURCES += extern/include/LptTools/LptTools_.cpp

# -----------------------------------------------------------------------
unix { 
    # LIBS += /usr/lib/libboost_thread.a \
    # /usr/lib/libboost_system.a
    LIBS += -lboost_thread \
        -lboost_system
    HARDWARE_PLATFORM = $$system(uname -m)
    contains( HARDWARE_PLATFORM, x86_64 ):: { 
        message(Building 64 bit )
        
        # 64-bit Linux
        LIBS += $$PWD/extern/lib/ticpp/linux/libticpp_64.a
    }
    else:: { 
        # 32-bit Linux
        message(Building 32 bit )
        LIBS += $$PWD/extern/lib/ticpp/linux/libticpp.a
    }
}
win32:LIBS += extern\lib\sdl\win\SDL.lib \
    extern\lib\sdl\win\SDLmain.lib \
    extern\lib\ticpp\win\ticpp.lib \
    extern\lib\g.usbamp\win\gUSBamp.lib \
    extern\lib\libgdf\win\libgdf.lib \
    kernel32.lib \
    advapi32.lib
