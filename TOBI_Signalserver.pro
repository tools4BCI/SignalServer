
#-----------------------------------------------------------------------

TEMPLATE = app

CONFIG   = release thread warn_on static

DEFINES  += TIXML_USE_TICPP
#TIMING_TEST

TARGET = signalserver

DESTDIR = bin
OBJECTS_DIR = tmp

DEPENDPATH  +=  extern/lib/ticpp/linux extern/lib/ticpp/win lib/g.usbamp

INCLUDEPATH +=  . include
INCLUDEPATH +=  extern/include
#INCLUDEPATH +=  src/config src/datapacket src/definitions src/hardware
#INCLUDEPATH +=  src/network src/sampleblock src/signalserver

windows: INCLUDEPATH  +=   include/SDL-1.2.14-VC8/include

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
            extern/include/LptTools\LptTools.h

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
  unix:LIBS  += -L extern/lib/ticpp/linux \
			  -lticppd \
			  -lboost_thread \
			  -lboost_system \
			  -lSDL
  #unix:TARGETDEPS   = extern/lib/ticpp/linux/libticppd.a
}

release {
  unix:LIBS  += -L extern/lib/ticpp/linux \
        -lticpp\
        -lboost_thread \
        -lboost_system \
        -lSDL
  #unix:TARGETDEPS   = extern/lib/ticpp/linux/libticpp.a
}

#-----------------------------------------------------------------------
