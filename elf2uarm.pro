#configure.target = config.h
#configure.commands = autoreconf; ./configure
#QMAKE_EXTRA_TARGETS += configure
#PRE_TARGETDEPS = config.h

QMAKE_CXXFLAGS += -std=c++11
QMAKE_LIBS += -lelf
TARGET = elf2uarm

HEADERS += \
    services/util.h \
    services/debug.h \
    armProc/blockdev_params.h \
    armProc/aout.h

SOURCES += \
    services/elf2arm.cc
