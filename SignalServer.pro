#-----------------------------------------------------------------------

TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += SignalServer_server.pro \
           TiA_client.pro \
           EEG_Simulator_client.pro \
           SignalServer_tests.pro

#-----------------------------------------------------------------------
#! end of file
