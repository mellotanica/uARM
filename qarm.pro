configure.target = config.h
configure.commands = autoreconf; ./configure
QMAKE_EXTRA_TARGETS += configure
PRE_TARGETDEPS = config.h

QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS += -lelf
CONFIG += qt debug
TARGET = uarm
#TARGET = elf2uarm

#INCLUDEPATH += /usr/include/sigc++-2.0 \
#	    /usr/lib/sigc++-2.0/include \
#	    /usr/lib/i386-linux-gnu/sigc++-2.0/include

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
    armProc/aout.h \
    services/debug.h \
    assemble_BIOS \
    config.h \
    armProc/device.h \
    armProc/machine_config.h \
    services/json.h \
    services/error.h \
    armProc/arch.h \
    services/lang.h \
    armProc/blockdev.h \
    armProc/vde_network.h \
    armProc/time_stamp.h \
    armProc/libvdeplug_dyn.h \
    armProc/event.h \
    services/utility.h \
    qarm/mac_id_edit.h \
    qarm/address_line_edit.h \
    qarm/machine_config_dialog_priv.h \
    qarm/machine_config_dialog.h

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
    qarm/hex_view.cc \
#    services/elf2arm.cc
    armProc/device.cc \
    armProc/machine_config.cc \
    services/json.cc \
    armProc/blockdev.cc \
    armProc/time_stamp.cc \
    armProc/vde_network.cc \
    armProc/event.cc \
    services/utility.cc \
    services/error.cc \
    qarm/mac_id_edit.cc \
    qarm/address_line_edit.cc \
    qarm/machine_config_dialog.cc

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
    test/BIOS_Base.bin \
    elftest/simpleBoot.s \
    icons/machine_settings-22.png
