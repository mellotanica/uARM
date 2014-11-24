QMAKE_CXXFLAGS += -std=c++11
QMAKE_LIBS += -lelf
TARGET = elf2uarm

DEFINES += UARM_MACHINE_COMPILING

HEADERS += \
    services/util.h \
    services/debug.h \
    armProc/blockdev_params.h \
    armProc/aout.h

SOURCES += \
    services/elf2arm.cc
