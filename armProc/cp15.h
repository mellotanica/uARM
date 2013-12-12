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

class cp15 : public coprocessor {
public:
    cp15(systemBus *bus);
    ~cp15();
	
    void reset();

    Word *getRegister(Word i) {return &cp15_registers[i];}

    Word *getRegList(){return cp15_registers;}
	
    void executeOperation(Byte opcode, Byte rm, Byte rn, Byte rd, Byte info);
    void registerTransfer(Word *cpuReg, Byte opcode, Byte operand, Byte srcDest, Byte info, bool toCoproc);
	
    Word *getIPCauseRegister();

private:
    enum ID_Codes{
        MAIN_ID = 0x41807767,
        CACHE_TYPE = 0,
        TCM_TYPE = 0,    //tightly coupled memory
        TLB_TYPE = 0,
        MPU_TYPE = 0
    };

    void register0(Word *cpureg, Byte opcode, Byte operand, Byte info, bool toCoproc);
    void register1(Word *cpureg, Byte info, bool toCoproc);
    void register2(Word *cpureg, Byte opcode, Byte info, bool toCoproc);
    void register15(Word *cpureg, Byte opcode, bool toCoproc);

    void EntryHi(Word *cpureg, Byte opcode, bool toCoproc);
    void EntryLo(Word *cpureg, bool toCoproc);

    bool getPFN();
	
    Word reg1CPmask;
	Word cp15_registers[CP15_REGISTERS_NUM];

    void writeCause(Word causeW);
};

#endif //UARM_CP15_CC
