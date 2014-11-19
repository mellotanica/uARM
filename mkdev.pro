QMAKE_CXXFLAGS += -std=c++11
QMAKE_LIBS += -lelf
TARGET = uarm-mkdev

HEADERS += \
    armProc/blockdev_params.h \
    armProc/const.h \
    armProc/types.h

SOURCES += \
    services/mkdev.cc
