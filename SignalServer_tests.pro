TEMPLATE += app
CONFIG += console
DESTDIR = bin
TARGET = test
OBJECTS_DIR = tmp/tests/

QT -= core \
      gui

INCLUDEPATH += $$PWD/. \
    $$PWD/include \
    $$PWD/extern/include
DEPENDPATH += $$INCLUDEPATH

DEFINES += TIXML_USE_TICPP
CONFIG( debug, debug|release ) {
    DEFINES += DEBUG
} else {

}



#QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
#QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage
#LIBS += -lgcov

QMAKE_CXXFLAGS_WARN_ON = -Wall \
    -pedantic

# ---------------------------

HEADERS += include/hardware/hw_thread.h


SOURCES += \
    tests/main.cpp \
    tests/sampleblock_tests.cpp\
    src/hardware/hw_thread.cpp \

win32 {
    SOURCES += tests/nirscout_tests.cpp\
               src/hardware/nirscout.cpp
}

# ---------------------------

LIBS += -lboost_thread \
        -lboost_system \
        -lboost_filesystem \
        -L$$PWD/extern/lib \
        -L$$PWD/extern/lib/ticpp/linux \
        -L$$PWD/tests/UnitTest++

HARDWARE_PLATFORM = $$system(uname -m)
  contains( HARDWARE_PLATFORM, x86_64 )::{
  # 64-bit Linux
  LIBS += -Lextern/lib/ticpp/linux  -lticpp_64 \
                -Lextern/lib/tia/linux/amd64 -ltia

  }else::{

  # 32-bit Linux
  LIBS += -Lextern/lib/ticpp/linux  -lticpp \
                -Lextern/lib/tia/linux/x86 -ltia
  }


contains( HARDWARE_PLATFORM, x86_64 )::{
  LIBS += -lUnitTest++_64
  }
  else:: {
  LIBS += -lUnitTest++
  }

