#-----------------------------------------------------------------------

include (TOBI_SignalServer_clientlib.pro)

CONFIG += dll

OBJECTS_DIR = tmp/ssclientlib_shared

win32:DEFINES += DECL_EXPORT=__declspec(dllexport)

#-----------------------------------------------------------------------
#! end of file
