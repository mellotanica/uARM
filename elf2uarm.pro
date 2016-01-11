CONFIG += c++11
QMAKE_LIBS += -lelf
TARGET = elf2uarm

DEFINES += UARM_MACHINE_COMPILING

HEADERS += \
    services/util.h \
    services/debug.h \
    armProc/blockdev_params.h \
    armProc/aout.h \
    facilities/arch.h

SOURCES += \
    services/elf2arm.cc

macx {
    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/local/lib
    CONFIG -= app_bundle
    DEFINES += MACOS_BUILD
}
