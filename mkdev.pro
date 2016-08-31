CONFIG += c++11 silent
QMAKE_LIBS += -lelf
TARGET = uarm-mkdev

DEFINES += UARM_MACHINE_COMPILING
DEFINES += MKDEV_BUILD

HEADERS += \
    armProc/blockdev_params.h \
    armProc/const.h \
    armProc/types.h \
    armProc/aout.h \
    services/elf2arm_mkdev.h

SOURCES += \
    services/mkdev.cc \
    services/elf2arm_mkdev.cc

macx {
    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/local/lib
    CONFIG -= app_bundle
    DEFINES += MACOS_BUILD
}
