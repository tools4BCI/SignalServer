
#-----------------------------------------------------------------------

TEMPLATE = app

CONFIG   = release thread warn_on static exceptions stl

DEFINES  += TIXML_USE_TICPP
#TIMING_TEST

TARGET = signalserver-sclient

DESTDIR = bin
OBJECTS_DIR = tmp



INCLUDEPATH +=  . include
INCLUDEPATH +=  extern/include
#INCLUDEPATH +=  src/config src/datapacket src/definitions
#INCLUDEPATH +=  src/sampleblock src/signalserver-client

DEPENDPATH  =   $$INCLUDEPATH
DEPENDPATH  +=  extern/lib/ticpp/linux  extern/lib/ticpp/win


#unix: QMAKE_CXXFLAGS += -O3

#-----------------------------------------------------------------------

# Input
HEADERS +=  include/signalserver-client/ssclientimpl_base.h\
            include/signalserver-client/ssclientimpl.h \
            include/signalserver-client/ssclient.h \
            include/definitions/constants.h \
            include/config/control_message_decoder.h \
            include/config/control_message_encoder.h \
            include/config/control_messages.h \
            include/config/ss_meta_info.h \
            include/config/xml_parser.h \
            include/datapacket/data_packet.h \
            include/datapacket/raw_mem.h \
            extern/include/LptTools\LptTools.h

SOURCES +=  src/signalserver-client/ssclient_main.cpp \
            src/signalserver-client/ssclientimpl.cpp \
            src/signalserver-client/ssclient.cpp \
            src/definitions/constants.cpp \
            src/config/control_message_decoder.cpp \
            src/config/control_message_encoder.cpp \
            src/config/control_messages.cpp \
            src/config/ss_meta_info.cpp \
            src/config/xml_parser.cpp \
            src/datapacket/data_packet.cpp \
            src/datapacket/raw_mem.cpp

unix:     SOURCES += extern/include/LptTools/LptToolsLinux.cpp
windows:  SOURCES += extern/include/LptTools/LptTools.cpp

#-----------------------------------------------------------------------

debug {
  unix:LIBS  += -L extern/lib/ticpp/linux \
        -lticppd \
        -lboost_thread \
        -lboost_system \
        -lSDL
  #unix:TARGETDEPS   = lib/ticpp/linux/libticppd.a

  win32:LIBS += extern\lib\sdl\win\SDL.lib \
                extern\lib\sdl\win\SDLmain.lib \
                extern\lib\ticpp\win\ticppd.lib \
                extern\lib\g.usbamp\win\gUSBamp.lib
}

release {
  unix:LIBS  += -L extern/lib/ticpp/linux \
        -lticpp\
        -lboost_thread \
        -lboost_system \
        -lSDL
  #unix:TARGETDEPS   = extern/lib/ticpp/linux/libticpp.a


  win32:LIBS += extern\lib\sdl\win\SDL.lib \
                extern\lib\sdl\win\SDLmain.lib \
                extern\lib\ticpp\win\ticpp.lib \
                extern\lib\g.usbamp\win\gUSBamp.lib
}

#-----------------------------------------------------------------------
