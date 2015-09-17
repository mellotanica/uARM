QMAKE_CXXFLAGS += -std=c++11
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

macx:INCLUDEPATH += -I /usr/local/include
macx:LIBS += -L/usr/local/lib
macx:LIBS += -L/usr/local/lib
macx:CONFIG-=app_bundle
macx:DEFINES += MACOS_BUILD
