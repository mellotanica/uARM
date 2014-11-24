QMAKE_CXXFLAGS += -std=c++11
QMAKE_LIBS += -lelf
TARGET = uarm-mkdev

DEFINES += UARM_MACHINE_COMPILING

HEADERS += \
    armProc/blockdev_params.h \
    armProc/const.h \
    armProc/types.h

SOURCES += \
    services/mkdev.cc
