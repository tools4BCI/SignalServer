#-----------------------------------------------------------------------

TEMPLATE = lib
VERSION = 0.1

CONFIG   += release static thread warn_on exceptions stl

QT -= core gui

DEFINES  += TIXML_USE_TICPP

win32:DEFINES += DECL_EXPORT=__declspec(dllexport)

DESTDIR = lib
TARGET = tiaclient

INCLUDEPATH += . include extern/include/LptTools

DEPENDPATH  +=  $$INCLUDEPATH

INCLUDEPATH += extern/include

QMAKE_CXXFLAGS_WARN_ON = -Wall -pedantic

# Input
HEADERS +=  include/config/control_message_decoder.h \
            include/config/control_message_encoder.h \
            include/config/control_messages.h \
            include/config/ss_meta_info.h \
            include/datapacket/data_packet.h \
            include/datapacket/raw_mem.h \
            include/definitions/constants.h \
            include/signalserver-client/ssclient.h \
            include/signalserver-client/ssclientimpl.h \
            include/signalserver-client/ssclientimpl_base.h \
            include/signalserver-client/ssconfig.h
#include/config/xml_parser.h \

SOURCES +=  src/signalserver-client/ssclientimpl.cpp \
            src/signalserver-client/ssclient.cpp \
            src/config/control_messages.cpp \
            src/config/control_message_decoder.cpp \
            src/config/control_message_encoder.cpp \
            src/config/ss_meta_info.cpp \
            src/datapacket/data_packet.cpp \
            src/datapacket/raw_mem.cpp \
            src/definitions/constants.cpp
#src/config/xml_parser.cpp \

unix:     SOURCES += extern/include/LptTools/LptToolsLinux.cpp
windows:  SOURCES += extern/include/LptTools/LptTools_.cpp

unix {
    LIBS  += -lboost_thread \
             -lboost_system

    HARDWARE_PLATFORM = $$system(uname -m)

    contains( HARDWARE_PLATFORM, x86_64 ) {

        # 64-bit Linux
        message(Building 64 bit )
        LIBS  += -L extern/lib/ticpp/linux \
                 -lticpp_64
    }
    else {
        # 32-bit Linux
        message(Building 32 bit )
        LIBS  += -L extern/lib/ticpp/linux \
                 -lticpp
    }
}

win32 {
    LIBS += extern\lib\ticpp\win\ticpp.lib \
            kernel32.lib advapi32.lib

    # Note: It is assumed that the boost libraries can be automatically detected by the linker
    # through #pragma comment(lib, xxx) declarations in boost.
}

#-----------------------------------------------------------------------
#! end of file
