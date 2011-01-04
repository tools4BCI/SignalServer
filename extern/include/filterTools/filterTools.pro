# -------------------------------------------------
# Project created by QtCreator 2010-12-16T10:12:25
# -------------------------------------------------
QT -= core \
    gui
TARGET = filterTools
TEMPLATE = lib
CONFIG += staticlib
DEFINES += FILTERTOOLS_LIBRARY
INCLUDEPATH += spuc/generic/
SOURCES += BasicFilter.cpp \
    iirnotch2.cpp \
    iirnotch.cpp \
    iirchebylpf.cpp \
    iirbutterlpf.cpp
HEADERS += BasicFilter.h \
    filterTools_global.h \
    iirnotch2.h \
    iirnotch.h \
    iirchebylpf.h \
    iirbutterlpf.h
