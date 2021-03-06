# -----------------------------------------------------------------------

HEADERS += include/config/xml_parser.h \
    include/config/xml_tags.h \
    include/hardware/hw_access.h \
    include/hardware/hw_thread.h \
    include/hardware/sine_generator.h \
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
    include/hardware/gBSamp_base.h \
    include/hardware/kinect.h \
    include/filewriter/file_writer.h \
    include/filewriter/file_writer_data_types.h \
    include/filewriter/file_writer_impl_base.h\
    include/filewriter/gdf_writer_impl.h

#    include/hardware/keylogger_base.h
#    include/hardware/event_listener.h \
unix {
  HEADERS += include/hardware/mouse_linux.h\
             include/hardware/nidaqmx_wrapper.h \
             include/hardware/gBSamp_unix.h
#             include/hardware/keylogger_linux.h
}
win32 {
  HEADERS += include/hardware/mouse_win.h \
             include/hardware/brainampseries.h \
             include/hardware/gBSamp_win.h \
             include/hardware/gtec_usbamp_wrapper.h \
             include/hardware/usbamp.h \
             include/hardware/dataq_wrapper.h \
             include/hardware/dataq.h \
             include/hardware/nirscout.h \
             include/hardware/plux.h
#             include/hardware/keylogger_win.h
}
contains( DEFINES, TIMING_TEST )::HEADERS += extern/include/LptTools/LptTools.h

# ---------------------------

SOURCES += src/signalserver/main.cpp \
    src/config/xml_parser.cpp \
    src/hardware/hw_access.cpp \
    src/hardware/hw_thread.cpp \
    src/hardware/sine_generator.cpp \
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
    src/hardware/kinect.cpp \
    src/filewriter/file_writer.cpp \
    src/filewriter/file_writer_impl_base.cpp\
    src/filewriter/gdf_writer_impl.cpp
#    src/hardware/keylogger_base.cpp
#    src/hardware/event_listener.cpp \
unix {
  SOURCES += src/hardware/mouse_linux.cpp \
             src/hardware/gBSamp_unix.cpp
#             src/hardware/keylogger_linux.cpp
  contains( DEFINES, TIMING_TEST )::SOURCES += extern/include/LptTools/LptToolsLinux.cpp
}

win32 {
  SOURCES += src/hardware/mouse_win.cpp \
             src/hardware/gBSamp_win.cpp \
             src/hardware/brainampseries.cpp \
             src/hardware/usbamp.cpp\
             src/hardware/dataq.cpp\
             src/hardware/nirscout.cpp \
             src/hardware/plux.cpp
#             src/hardware/keylogger_win.cpp
  contains( DEFINES, TIMING_TEST )::SOURCES += extern/include/LptTools/LptTools_.cpp
}


