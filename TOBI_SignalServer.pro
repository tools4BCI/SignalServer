#-----------------------------------------------------------------------

TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += TOBI_SignalServer_tialib_static.pro \
           TOBI_SignalServer_tialib_shared.pro \
           TOBI_SignalServer_server.pro \
           TOBI_SignalServer_client.pro \
           TOBI_tia_tests.pro

#-----------------------------------------------------------------------
#! end of file
