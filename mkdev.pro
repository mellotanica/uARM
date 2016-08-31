QMAKE_CXXFLAGS += -std=c++11
QMAKE_LIBS += -lelf
TARGET = uarm-mkdev

DEFINES += UARM_MACHINE_COMPILING
DEFINES += MKDEV_BUILD

HEADERS += \
    armProc/blockdev_params.h \
    armProc/const.h \
    armProc/types.h \
    services/elf2arm.h

SOURCES += \
    services/mkdev.cc \
    services/elf2arm.cc
