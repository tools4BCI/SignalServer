#-----------------------------------------------------------------------

TEMPLATE = app

CONFIG   = console release thread warn_on static exceptions stl

DEFINES  += TIXML_USE_TICPP
#TIMING_TEST

TARGET = signalserver

DESTDIR = bin
OBJECTS_DIR = tmp

INCLUDEPATH +=  . include src
INCLUDEPATH +=  extern/include
INCLUDEPATH +=  /home/breidi/svn/BCI/HEAD/Common/gdf
#INCLUDEPATH +=  src/config src/datapacket src/definitions src/hardware
#INCLUDEPATH +=  src/network src/sampleblock src/signalserver

INCLUDEPATH += extern/include/SDL-1.2.14-VC8/include

DEPENDPATH  =   $$INCLUDEPATH
DEPENDPATH  +=  extern/lib/ticpp/linux extern/lib/ticpp/win lib/g.usbamp

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
            include/sampleblock/sample_block.h \
            include/config/control_message_decoder.h \
            include/config/control_message_encoder.h \
            include/config/control_messages.h \
            include/config/ss_meta_info.h \
            include/config/xml_parser.h \
            include/datapacket/data_packet.h \
            include/datapacket/raw_mem.h \
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
            src/sampleblock/sample_block.cpp \
            src/config/control_message_decoder.cpp \
            src/config/control_message_encoder.cpp \
            src/config/control_messages.cpp \
            src/config/ss_meta_info.cpp \
            src/config/xml_parser.cpp \
            src/datapacket/data_packet.cpp \
            src/datapacket/raw_mem.cpp \
            src/network/control_connection_server.cpp \
            src/network/tcp_data_server.cpp \
            src/network/tcp_server.cpp \
            src/network/udp_data_server.cpp


unix:     SOURCES += extern/include/LptTools/LptToolsLinux.cpp
windows:  SOURCES += extern/include/LptTools/LptTools.cpp

#-----------------------------------------------------------------------

debug {
  unix:LIBS  += -L extern/lib/ticpp/linux -L /home/breidi/svn/BCI/HEAD/Common/gdf/libgdf \
			  -lticppd \
			  -lboost_thread \
			  -lboost_system \
			  -lSDL \
                    -llibgdf
  #unix:TARGETDEPS   = extern/lib/ticpp/linux/libticppd.a

  win32:LIBS += extern\lib\sdl\win\SDL.lib \
                extern\lib\sdl\win\SDLmain.lib \
                extern\lib\ticpp\win\ticppd.lib \
                extern\lib\g.usbamp\win\gUSBamp.lib
}

release {
  unix:LIBS  += -L extern/lib/ticpp/linux -L /home/breidi/svn/BCI/HEAD/Common/gdf/libgdf \
        -lticpp\
        -lboost_thread \
        -lboost_system \
        -lSDL \
        -llibgdf
  #unix:TARGETDEPS   = extern/lib/ticpp/linux/libticpp.a

  win32:LIBS += extern\lib\sdl\win\SDL.lib \
                extern\lib\sdl\win\SDLmain.lib \
                extern\lib\ticpp\win\ticpp.lib \
                extern\lib\g.usbamp\win\gUSBamp.lib
}

#-----------------------------------------------------------------------
