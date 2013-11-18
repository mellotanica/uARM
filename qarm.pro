QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS += -lelf
CONFIG += qt debug
TARGET = uarm
#TARGET = elf2uarm

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
    qarm/mainbar.h \
    qarm/guiConst.h \
    qarm/procdisplay.h \
    qarm/QLine.h \
    qarm/ramview.h \
    qarm/hex_view_priv.h \
    qarm/hex_view.h \
    armProc/blockdev_params.h \
    armProc/aout.h

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
    qarm/procdisplay.cc \
    qarm/ramview.cc \
    qarm/hex_view.cc
#    services/elf2arm.cc

OTHER_FILES += \
    test/testProgBehavior \
    compile \
    assemble \
    icons/step.png \
    icons/reset.png \
    icons/play.png \
    icons/pause.png \
    test/sample.s \
    README.txt \
    test/simpleBoot.s \
    test/dataP_MOV_MVN_BIC.s \
    test/IsaCheck/dataP_MOV_MVN_BIC.s \
    test/IsaCheck/dataP_AND_ORR_EOR.s \
    test/IsaCheck/dataP_ADD_SUB_RSB.s \
    test/IsaCheck/dataP_ADC_SBC_RSC.s \
    test/IsaCheck/dataP_TST_TEQ_CMP_CMN.s \
    test/BIOS_Base.bin
