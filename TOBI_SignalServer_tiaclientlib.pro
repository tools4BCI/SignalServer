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
HEADERS +=  include/tia-private/config/control_message_decoder.h \
            include/tia-private/config/control_message_encoder.h \
            include/tia-private/config/control_messages.h \
            include/tia/ss_meta_info.h \
            include/tia/data_packet.h \
            include/tia-private/datapacket/raw_mem.h \
            include/tia/constants.h \
            include/tia/ssclient.h \
            include/tia-private/signalserver-client/ssclientimpl.h \
            include/tia-private/signalserver-client/ssclientimpl_base.h \
            include/tia/ssconfig.h

SOURCES +=  src/signalserver-client/ssclientimpl.cpp \
            src/signalserver-client/ssclient.cpp \
            src/tia/config/control_messages.cpp \
            src/tia/config/control_message_decoder.cpp \
            src/tia/config/control_message_encoder.cpp \
            src/tia/ss_meta_info.cpp \
            src/tia/data_packet.cpp \
            src/tia/datapacket/raw_mem.cpp \
            src/tia/constants.cpp

unix:     SOURCES += extern/include/LptTools/LptToolsLinux.cpp
windows:  SOURCES += extern/include/LptTools/LptTools_.cpp

unix {
#    LIBS += /usr/lib/libboost_thread.a \
#            /usr/lib/libboost_system.a

    LIBS += -lboost_thread \
            -lboost_system

    HARDWARE_PLATFORM = $$system(uname -m)

    contains( HARDWARE_PLATFORM, x86_64 ) {

        # 64-bit Linux
        message(Building 64 bit )
        LIBS  += extern/lib/ticpp/linux/libticpp_64.a
    }
    else {
        # 32-bit Linux
        message(Building 32 bit )
        LIBS  += extern/lib/ticpp/linux/libticpp.a
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
