QT += core gui widgets

TARGET = uarm

CONFIG += c++11
QMAKE_LIBS += -ldl
QMAKE_LIBS += -lelf
CONFIG += qt debug

DEFINES += UARM_MACHINE_COMPILING
#DEFINES += UARM_DUMMY_DISASSEMBLER

HEADERS += \
    armProc/aout.h \
    armProc/ARMisa.h \
    armProc/basic_types.h \
    armProc/blockdev.h \
    armProc/blockdev_params.h \
    armProc/bus.h \
    armProc/const.h \
    armProc/cp15.h \
    armProc/device.h \
    armProc/event.h \
    armProc/libvdeplug_dyn.h \
    armProc/machine.h \
    armProc/machine_config.h \
    armProc/mpic.h \
    armProc/processor.h \
    armProc/pu.h \
    armProc/ramMemory.h \
    armProc/registers.h \
    armProc/stoppoint.h \
    armProc/Thumbisa.h \
    armProc/time_stamp.h \
    armProc/tlbentry.h \
    armProc/types.h \
    armProc/vde_network.h \
    facilities/arch.h \
    facilities/uARMconst.h \
    facilities/uARMtypes.h \
    qarm/add_breakpoint_dialog.h \
    qarm/address_line_edit.h \
    qarm/breakpoint_window.h \
    qarm/flat_push_button.h \
    qarm/guiConst.h \
    qarm/hex_view.h \
    qarm/hex_view_priv.h \
    qarm/mainbar.h \
    qarm/mac_id_edit.h \
    qarm/machine_config_dialog.h \
    qarm/machine_config_dialog_priv.h \
    qarm/procdisplay.h \
    qarm/qarm.h \
    qarm/qarmmessagebox.h \
    qarm/QLine.h \
    qarm/ramview.h \
    qarm/register_item_delegate.h \
    qarm/select_structures_dialog.h \
    qarm/stoppoint_list_model.h \
    qarm/structures_window.h \
    qarm/symbol_table_model.h \
    qarm/terminal_view.h \
    qarm/terminal_window.h \
    qarm/terminal_window_priv.h \
    qarm/tlb_model.h \
    qarm/tlb_window.h \
    qarm/ui_utils.h \
    services/debug.h \
    services/debug_session.h \
    services/disass.h \
    services/error.h \
    services/json.h \
    services/lang.h \
    services/symbol_table.h \
    services/util.h \
    services/utility.h \
    services/elf2arm.h

SOURCES += \
    armProc/ARMisa.cc \
    armProc/blockdev.cc \
    armProc/bus.cc \
    armProc/cp15.cc \
    armProc/device.cc \
    armProc/event.cc \
    armProc/machine.cc \
    armProc/machine_config.cc \
    armProc/mpic.cc \
    armProc/processor.cc \
    armProc/stoppoint.cc \
    armProc/Thumbisa.cc \
    armProc/time_stamp.cc \
    armProc/tlbentry.cpp \
    armProc/vde_network.cc \
    qarm/add_breakpoint_dialog.cc \
    qarm/address_line_edit.cc \
    qarm/breakpoint_window.cc \
    qarm/flat_push_button.cc \
    qarm/hex_view.cc \
    qarm/mac_id_edit.cc \
    qarm/machine_config_dialog.cc \
    qarm/main.cc \
    qarm/mainbar.cc \
    qarm/procdisplay.cc \
    qarm/qarm.cc \
    qarm/qarmmessagebox.cpp \
    qarm/ramview.cc \
    qarm/register_item_delegate.cc \
    qarm/select_structures_dialog.cc \
    qarm/stoppoint_list_model.cc \
    qarm/structures_window.cc \
    qarm/symbol_table_model.cc \
    qarm/terminal_view.cc \
    qarm/terminal_window.cc \
    qarm/tlb_model.cc \
    qarm/tlb_window.cc \
    qarm/ui_utils.cc \
    services/debug.cc \
    services/debug_session.cc \
    services/disass.c \
    services/elf2arm.cc \
    services/error.cc \
    services/json.cc \
    services/symbol_table.cc \
    services/utility.cc

OTHER_FILES += \
    assemble_BIOS \
    compile \
    config.h.in \
    configure \
    configure.ac \
    default/uarm \
    elf2uarm.pro \
    facilities/BIOS.rom.uarm \
    facilities/BIOS.s \
    facilities/bios_const.h \
    facilities/crti.o \
    facilities/crti.s \
    facilities/crtso.o \
    facilities/crtso.s \
    facilities/libdiv.o \
    facilities/libdiv.s \
    facilities/libuarm.h \
    facilities/libuarm.o \
    facilities/libuarm.s \
    icons/machine_settings-22.png \
    icons/pause.png \
    icons/play.png \
    icons/poweron.png \
    icons/reset.png \
    icons/step.png \
    icons/window_default-48.png \
    install.sh \
    ldscript/elf32ltsarm.h.uarmaout.x \
    ldscript/elf32ltsarm.h.uarmcore.x \
    README.txt \
    test/dataP_MOV_MVN_BIC.s \
    test/helloWorld.c

DISTFILES += \
    todo.txt

macx {
    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/local/lib
    HEADERS += services/fmemopen.h
    SOURCES += services/fmemopen.c
    ICON = uarm.icns
    DEFINES += MACOS_BUILD
}
