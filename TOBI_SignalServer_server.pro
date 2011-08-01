# -----------------------------------------------------------------------
TEMPLATE += app
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
#        USE_TID_SERVER\
#        USE_GDF_SAVER

# TIMING_TEST
TARGET = signalserver
DESTDIR = $$PWD/bin
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
    PRE_TARGETDEPS += $$PWD/update-version.sh
    QMAKE_EXTRA_TARGETS += versiontarget
}
QMAKE_CXXFLAGS_WARN_ON = -Wall \
    -pedantic

# -----------------------------------------------------------------------
HEADERS += include/config/xml_parser.h \
    include/hardware/hw_access.h \
    include/hardware/hw_thread.h \
    include/hardware/sine_generator.h \
    include/hardware/event_listener.h \
    include/hardware/jstick.h \
    include/hardware/serial_port_base.h \
    include/hardware/gtec_usbamp_wrapper.h \
    include/hardware/g_mobilab.h \
    include/hardware/hw_thread_factory.h \
    include/hardware/hw_thread_builder.h \
    include/sampleblock/sample_block.h \
    include/hardware/artificial_signal_source.h \
    include/hardware/eeg_simulator.h \
    extern/include/LptTools/LptTools.h \
    include/hardware/mouse.h \
    include/hardware/eeg_sim_msg_parser.h \
    include/signalserver/signalserver.h\
    include/hardware/nidaqmx_wrapper.h \
    include/hardware/gBSamp_base.h \
    include/hardware/gBSamp_unix.h
    #include/hardware/gBSamp_win.h \
    #    include/filereading/data_file_handler.h \
    #    include/filereading/file_reader_factory.h \
    #    include/filereading/file_reader.h \
    #    include/filereading/gdf_file_reader.h \
    #include/hardware/nirscout.h
    #include/hardware/brainampseries.h
unix:HEADERS += include/hardware/mouse_linux.h
win32:HEADERS += include/hardware/mouse_win.h
SOURCES += src/signalserver/main.cpp \
    src/config/xml_parser.cpp \
    src/hardware/hw_access.cpp \
    src/hardware/hw_thread.cpp \
    src/hardware/sine_generator.cpp \
    src/hardware/event_listener.cpp \
    src/hardware/jstick.cpp \
    src/hardware/serial_port_base.cpp \
    src/hardware/g_mobilab.cpp \
    src/hardware/hw_thread_factory.cpp \
    src/sampleblock/sample_block.cpp \
    src/hardware/artificial_signal_source.cpp \
    src/hardware/eeg_simulator.cpp \
    src/hardware/mouse.cpp \
    src/hardware/eeg_sim_msg_parser.cpp \
    src/signalserver/signalserver.cpp \
    src/hardware/gBSamp_base.cpp \
    src/hardware/gBSamp_unix.cpp
    #src/hardware/gBSamp_win .cpp
    #    src/filereading/data_file_handler.cpp \
    #    src/filereading/file_reader_factory.cpp \
    #    src/filereading/file_reader.cpp \
    #    src/filereading/gdf_file_reader.cpp \
    #src/hardware/nirscout.cpp
    #src/hardware/brainampseries.cpp
unix:SOURCES += src/hardware/mouse_linux.cpp
win32:SOURCES += src/hardware/mouse_win.cpp
unix:SOURCES += extern/include/LptTools/LptToolsLinux.cpp
win32:SOURCES += extern/include/LptTools/LptTools_.cpp
LIBS += $$PWD/lib/libtia.a

# -----------------------------------------------------------------------
unix {
    # LIBS += /usr/lib/libboost_thread.a \
    # /usr/lib/libboost_system.a \
    # /usr/lib/libboost_filesystem.a \
    # -lSDL
    LIBS += -lboost_thread \
        -lboost_system \
        -lboost_filesystem \
        -lboost_program_options\
        -lSDL \
        -lusb-1.0 \
        -ltobicore \
        -ltobiid \
        -lcomedi
    HARDWARE_PLATFORM = $$system(uname -m)
    contains( HARDWARE_PLATFORM, x86_64 ):: {
        message(Building 64 bit )

        # 64-bit Linux
        LIBS += -L \
            $$PWD/extern/lib/ticpp/linux \
            -lticpp_64\
            $$PWD/extern/lib/TiDlib/linux/libTiDserver_64.a\
            $$PWD/extern/lib/libgdf/linux/libGDF_64.a
    }else::{
        # 32-bit Linux
        message(Building 32 bit )
        LIBS += -L$$PWD/extern/lib/ticpp/linux \
            -lticpp\
            $$PWD/extern/lib/TiDlib/linux/libTiDserver.a\
            $$PWD/extern/lib/libgdf/linux/libGDF_64.a
    }
}

# -Lextern/lib/libgdf/linux \
# -llibgdf
win32:LIBS += extern\lib\sdl\win\SDL.lib \
    extern\lib\sdl\win\SDLmain.lib \
    extern\lib\ticpp\win\ticpp.lib \
    extern\lib\g.usbamp\win\gUSBamp.lib \
    extern\lib\libgdf\win\libgdf.lib \
    kernel32.lib \
    advapi32.lib
