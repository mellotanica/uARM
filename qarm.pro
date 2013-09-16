QMAKE_CXXFLAGS += -std=c++11
CONFIG += qt debug
TARGET = uarm

HEADERS += \
    armProc/types.h \
    armProc/Thumbisa.h \
    armProc/registers.h \
    armProc/ramMemory.h \
    armProc/pu.h \
    armProc/processor.h \
    armProc/machine.h \
    armProc/cp15.h \
    armProc/coprocessor_interface.h \
    armProc/const.h \
    armProc/bus.h \
    armProc/basic_types.h \
    armProc/ARMisa.h \
    qarm/qarm.h \
    services/util.h \
    services/lang.h \
    qarm/mainbar.h \
    qarm/guiConst.h \
    qarm/procdisplay.h

SOURCES += \
    armProc/Thumbisa.cc \
    armProc/processor.cc \
    armProc/machine.cc \
    armProc/cp15.cc \
    armProc/coprocessor_interface.cc \
    armProc/bus.cc \
    armProc/ARMisa.cc \
    qarm/qarm.cc \
    qarm/main.cc \
    qarm/mainbar.cc \
    qarm/procdisplay.cc

OTHER_FILES += \
    test/testProgBehavior \
    compile \
    assemble
