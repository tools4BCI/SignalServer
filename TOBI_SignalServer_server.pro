#-----------------------------------------------------------------------

TEMPLATE += app

CONFIG   += console release thread warn_on static exceptions stl

QT -= core gui

DEFINES  += TIXML_USE_TICPP
#TIMING_TEST

TARGET = signalserver

DESTDIR = bin
OBJECTS_DIR = tmp

INCLUDEPATH += . include extern/include/LptTools

DEPENDPATH  +=  $$INCLUDEPATH

INCLUDEPATH += extern/include

win32:INCLUDEPATH += extern/include/SDL-1.2.14-VC8

#unix: QMAKE_CXXFLAGS += -O3

#-----------------------------------------------------------------------

HEADERS +=  include/signalserver/signal_server.h \
            include/definitions/constants.h \
            include/hardware/hw_access.h \
            include/hardware/hw_thread.h \
            include/hardware/sine_generator.h \
            include/hardware/event_listener.h\
            include/hardware/jstick.h\
            include/hardware/usbamp.h\
            include/filereading/data_file_handler.h\
            include/filereading/file_reader_factory.h\
            include/filereading/file_reader.h\
            include/filereading/gdf_file_reader.h\
            include/sampleblock/sample_block.h \
            include/config/control_message_decoder.h \
            include/config/control_message_encoder.h \
            include/config/control_messages.h \
            include/config/ss_meta_info.h \
            include/config/xml_parser.h \
            include/datapacket/data_packet.h \
            include/datapacket/raw_mem.h \
            include/network/control_connection.h \
            include/network/control_connection_server.h \
            include/network/tcp_data_server.h \
            include/network/tcp_server.h \
            include/network/udp_data_server.h \
            extern/include/LptTools/LptTools.h

SOURCES +=  src/signalserver/main.cpp \
            src/signalserver/signal_server.cpp \
            src/definitions/constants.cpp \
            src/hardware/hw_access.cpp \
            src/hardware/hw_thread.cpp \
            src/hardware/sine_generator.cpp \
            src/hardware/event_listener.cpp\
            src/hardware/jstick.cpp\
            src/hardware/usbamp.cpp\
            src/filereading/data_file_handler.cpp\
            src/filereading/file_reader_factory.cpp\
            src/filereading/file_reader.cpp\
            src/filereading/gdf_file_reader.cpp\
            src/sampleblock/sample_block.cpp \
            src/config/control_message_decoder.cpp \
            src/config/control_message_encoder.cpp \
            src/config/control_messages.cpp \
            src/config/ss_meta_info.cpp \
            src/config/xml_parser.cpp \
            src/datapacket/data_packet.cpp \
            src/datapacket/raw_mem.cpp \
            src/network/control_connection.cpp \
            src/network/control_connection_server.cpp \
            src/network/tcp_data_server.cpp \
            src/network/tcp_server.cpp \
            src/network/udp_data_server.cpp

unix:SOURCES  += extern/include/LptTools/LptToolsLinux.cpp
win32:SOURCES += extern/include/LptTools/LptTools_.cpp

#-----------------------------------------------------------------------

unix {
    LIBS  += -lboost_thread \
             -lboost_system \
             -lSDL

    HARDWARE_PLATFORM = $$system(uname -i)

    contains( HARDWARE_PLATFORM, x86_64 ) {
        # 64-bit Linux
        LIBS  += -L extern/lib/ticpp/linux \
                 -lticpp_64 \
                 -Lextern/lib/libgdf/linux \
                 -lgdf
    }
    else {
        # 32-bit Linux
        LIBS  += -Lextern/lib/ticpp/linux \
                 -lticpp \
                 -Lextern/lib/libgdf/linux \
                 -llibgdf
    }
}

win32 {
    LIBS += extern\lib\sdl\win\SDL.lib \
            extern\lib\sdl\win\SDLmain.lib \
            extern\lib\ticpp\win\ticpp.lib \
            extern\lib\g.usbamp\win\gUSBamp.lib \
            extern\lib\libgdf\win\libgdf.lib \
            kernel32.lib advapi32.lib

    # Note: It is assumed that the boost libraries can be automatically detected by the linker
    # through #pragma comment(lib, xxx) declarations in boost.
}

unix {
    # TODO:
#    exists( /home/breidi/svn/BCI/HEAD/Common/gdf ) {
#        INCLUDEPATH +=  /home/breidi/svn/BCI/HEAD/Common/gdf

#        LIBS += -L/home/breidi/svn/BCI/HEAD/Common/gdf/libgdf \
#                -llibgdf
#        LIBS += -llibgdf

#    }
}

#-----------------------------------------------------------------------
#! end of file