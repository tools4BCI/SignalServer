# -----------------------------------------------------------------------

HEADERS += include/config/xml_parser.h \
    include/hardware/hw_access.h \
    include/hardware/hw_thread.h \
    include/hardware/sine_generator.h \
    include/hardware/event_listener.h \
    include/hardware/jstick.h \
    include/hardware/serial_port_base.h \
    include/hardware/g_mobilab.h \
    include/hardware/hw_thread_factory.h \
    include/hardware/hw_thread_builder.h \
    include/sampleblock/sample_block.h \
    include/hardware/artificial_signal_source.h \
    include/hardware/eeg_simulator.h \
    include/hardware/mouse.h \
    include/hardware/eeg_sim_msg_parser.h \
    include/signalserver/signalserver.h \
    include/hardware/gBSamp_base.h

unix {
  HEADERS += include/hardware/mouse_linux.h\
             include/hardware/nidaqmx_wrapper.h \
             include/hardware/gBSamp_unix.h
}
win32 {
  HEADERS += include/hardware/mouse_win.h \
             include/hardware/brainampseries.h \
             include/hardware/gBSamp_win.h \
             include/hardware/gtec_usbamp_wrapper.h \
             include/hardware/usbamp.h
}
contains( DEFINES, TIMING_TEST )::HEADERS += extern/include/LptTools/LptTools.h

# ---------------------------

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
    src/hardware/gBSamp_base.cpp

unix {
  SOURCES += src/hardware/mouse_linux.cpp \
             src/hardware/gBSamp_unix.cpp
  contains( DEFINES, TIMING_TEST )::SOURCES += extern/include/LptTools/LptToolsLinux.cpp
}

win32 {
  SOURCES += src/hardware/mouse_win.cpp \
             src/hardware/gBSamp_win.cpp \
             src/hardware/brainampseries.cpp \
             src/hardware/usbamp.cpp
  contains( DEFINES, TIMING_TEST )::SOURCES += extern/include/LptTools/LptTools_.cpp
}


