QT += core gui declarative

TARGET = uarm

QMAKE_CXXFLAGS += -std=c++11
QMAKE_LIBS += -ldl
CONFIG += qt debug

HEADERS += \
    armProc/types.h \
    armProc/Thumbisa.h \
    armProc/registers.h \
    armProc/ramMemory.h \
    armProc/pu.h \
    armProc/processor.h \
    armProc/machine.h \
    armProc/cp15.h \
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
    armProc/device.h \
    armProc/machine_config.h \
    services/json.h \
    services/error.h \
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
    qarm/machine_config_dialog.h \
    armProc/mpic.h \
    qarm/terminal_window_priv.h \
    qarm/terminal_window.h \
    qarm/terminal_view.h \
    qarm/flat_push_button.h \
    qarm/breakpoint_window.h \
    facilities/arch.h \
    configUbuntu.h \
    services/disass.h \
    qarm/symbol_table_model.h \
    qarm/add_breakpoint_dialog.h \
    services/symbol_table.h \
    services/debug_session.h \
    qarm/stoppoint_list_model.h \
    armProc/stoppoint.h \
    qarm/ui_utils.h \
    facilities/uARMconst.h \
    facilities/uARMtypes.h

SOURCES += \
    armProc/Thumbisa.cc \
    armProc/processor.cc \
    armProc/machine.cc \
    armProc/cp15.cc \
    armProc/bus.cc \
    armProc/ARMisa.cc \
    qarm/qarm.cc \
    qarm/main.cc \
    qarm/mainbar.cc \
    qarm/procdisplay.cc \
    qarm/ramview.cc \
    qarm/hex_view.cc \
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
    qarm/machine_config_dialog.cc \
    armProc/mpic.cc \
    qarm/terminal_window.cc \
    qarm/terminal_view.cc \
    qarm/flat_push_button.cc \
    services/debug.cc \
    services/disass.c \
    qarm/breakpoint_window.cc \
    qarm/symbol_table_model.cc \
    qarm/add_breakpoint_dialog.cc \
    services/symbol_table.cc \
    services/debug_session.cc \
    qarm/stoppoint_list_model.cc \
    armProc/stoppoint.cc \
    qarm/ui_utils.cc

RESOURCES +=

OTHER_FILES += \
    compile \
    assemble_BIOS \
    icons/step.png \
    icons/reset.png \
    icons/play.png \
    icons/pause.png \
    README.txt \
    test/dataP_MOV_MVN_BIC.s \
    icons/machine_settings-22.png \
    elf2uarm.pro \
    facilities/BIOS.s \
    facilities/bios_const.h \
    facilities/libuarm.h \
    facilities/libuarm.o \
    facilities/crtso.S \
    facilities/libuarm.S \
    ldscript/elf32ltsarm.h.uarmcore.x \
    ldscript/elf32ltsarm.h.uarmaout.x \
    configure \
    configure.ac \
    config.h.in \
    test/helloWorld.c \
    facilities/crtso.o \
    facilities/BIOS.rom.uarm \
    icons/window_default-48.png \
    icons/poweron.png \
    install.sh \
    default/uarm
