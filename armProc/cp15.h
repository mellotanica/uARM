/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uARM
 *
 * Copyright (C) 2013 Marco Melletti
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef UARM_CP15_H
#define UARM_CP15_H

#include "armProc/const.h"
#include "armProc/pu.h"
#include "armProc/bus.h"

/*
 * CP0 EntryHi fields
 */
#define ENTRYHI_SEGNO_MASK     0xc0000000
#define ENTRYHI_SEGNO_BIT      30
#define ENTRYHI_GET_SEGNO(x)   (((x) & ENTRYHI_SEGNO_MASK) >> ENTRYHI_SEGNO_BIT)

#define ENTRYHI_VPN_MASK       0x3ffff000
#define ENTRYHI_VPN_BIT        12
#define ENTRYHI_GET_VPN(x)     (((x) & ENTRYHI_VPN_MASK) >> ENTRYHI_VPN_BIT)

#define ENTRYHI_ASID_MASK      0x00000fe0
#define ENTRYHI_ASID_BIT       5
#define ENTRYHI_GET_ASID(x)    (((x) & ENTRYHI_ASID_MASK) >> ENTRYHI_ASID_BIT)

/*
 * CP0 EntryLo fields
 */
#define ENTRYLO_PFN_MASK       0xfffff000
#define ENTRYLO_PFN_BIT        12
#define ENTRYLO_GET_PFN(x)     (((x) & ENTRYLO_PFN_MASK) >> ENTRYLO_PFN_BIT)

#define ENTRYLO_DIRTY          0x00000400
#define ENTRYLO_DIRTY_BIT      10
#define ENTRYLO_VALID          0x00000200
#define ENTRYLO_VALID_BIT      9
#define ENTRYLO_GLOBAL         0x00000100
#define ENTRYLO_GLOBAL_BIT     8

class cp15 : public coprocessor {
public:
    cp15(systemBus *bus);
    ~cp15();
	
    void reset();

    Word *getRegister(Word i) {return &cp15_registers[i];}

    Word *getRegList(){return cp15_registers;}
	
    bool executeOperation(Byte opcode, Byte rm, Byte rn, Byte rd, Byte info);
    bool registerTransfer(Word *cpuReg, Byte opcode, Byte operand, Byte srcDest, Byte info, bool toCoproc);
	
    Word *getIPCauseRegister();

    void setCause(unsigned int cause);

    bool isVMon(){return (cp15_registers[CP15_REG1_SCB] & (1 << CP15_REG1_MPOS));}

    void clockTick();

private:
    enum ID_Codes{
        MAIN_ID = 0x41807767,
        CACHE_TYPE = 0,
        TCM_TYPE = 0,    //tightly coupled memory
        TLB_TYPE = 0,
        MPU_TYPE = 0
    };

    void register0(Word *cpureg, Byte opcode, Byte operand, Byte info, bool toCoproc);
    bool register1(Word *cpureg, Byte info, bool toCoproc);
    bool register2(Word *cpureg, Byte opcode, Byte info, bool toCoproc);
    void register15(Word *cpureg, Byte opcode, bool toCoproc);

    void genRegister(Word *cpureg, Word cpreg, bool toCoproc);

    bool EntryHi(Word *cpureg, Byte opcode, bool toCoproc);
    void EntryLo(Word *cpureg, bool toCoproc);

    bool getPFN();
	
    Word reg1CPmask;
	Word cp15_registers[CP15_REGISTERS_NUM];

    void writeCause(Word causeW);
};

#endif //UARM_CP15_CC
