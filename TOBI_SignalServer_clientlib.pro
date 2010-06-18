TEMPLATE = lib

CONFIG   = release staticlib thread warn_on exceptions stl

DEFINES  += TIXML_USE_TICPP

DESTDIR = lib
TARGET = ssclient

INCLUDEPATH += . include src
INCLUDEPATH += extern/include

DEPENDPATH  =   $$INCLUDEPATH
DEPENDPATH  +=  extern/lib/ticpp/linux  extern/lib/ticpp/win

#unix: QMAKE_CXXFLAGS += -O3

# Input
HEADERS +=  include/config/control_message_decoder.h \
            include/config/control_message_encoder.h \
            include/config/control_messages.h \
            include/config/ss_meta_info.h \
            include/config/xml_parser.h \
            include/datapacket/data_packet.h \
            include/datapacket/raw_mem.h \
            include/definitions/constants.cpp \
            include/signalserver-client/ssclient.h \
            include/signalserver-client/ssclientimpl.h \
            include/signalserver-client/ssclientimpl_base.h

SOURCES +=  src/signalserver-client/ssclient_main.cpp \
            src/signalserver-client/ssclientimpl.cpp \
            src/signalserver-client/ssclient.cpp \
            src/config/xml_parser.cpp \
            src/config/control_messages.cpp \
            src/config/control_message_decoder.cpp \
            src/config/control_message_encoder.cpp \
            src/config/ss_meta_info.cpp \
            src/datapacket/data_packet.cpp \
            src/datapacket/raw_mem.cpp \
            src/definitions/constants.cpp
debug {
  unix:LIBS  += -L ../lib_ticpp \
               -lticppd \
               -lboost_thread-d \
               -lboost_system-d
#unix:TARGETDEPS   = ../lib_ticpp/libticppd.a


  win32:LIBS += extern\lib\sdl\win\SDL.lib \
                extern\lib\sdl\win\SDLmain.lib \
                extern\lib\ticpp\win\ticppd.lib \
                extern\lib\g.usbamp\win\gUSBamp.lib
}

release {
  unix:LIBS  += -L ../lib_ticpp \
        -lticpp \
        -lboost_thread \
        -lboost_system
#unix:TARGETDEPS   = extern/lib/libticpp.a

  win32:LIBS += extern\lib\sdl\win\SDL.lib \
                extern\lib\sdl\win\SDLmain.lib \
                extern\lib\ticpp\win\ticpp.lib \
                extern\lib\g.usbamp\win\gUSBamp.lib
}

#unix:TARGETDEPS   = ../lib_ticpp/libticppd.a
