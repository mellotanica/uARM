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

#ifndef UARM_CP15_CC
#define UARM_CP15_CC

#include "cp15.h"

cp15::cp15(systemBus *bus): coprocessor(bus) {
    reset();
}

cp15::~cp15(){
    delete [] cp15_registers;
}

void cp15::reset(){
    for(int i = 0; i < CP15_REGISTERS_NUM; i++)
        cp15_registers[i] = 0;
    reg1CPmask = 0;
    #ifdef COPROCESSOR_CP0
        reg1CPmask |= 3;
    #endif
    #ifdef COPROCESSOR_CP1
        reg1CPmask |= 3 << 2;
    #endif
    #ifdef COPROCESSOR_CP2
        reg1CPmask |= 3 << 4;
    #endif
    #ifdef COPROCESSOR_CP3
        reg1CPmask |= 3 << 6;
    #endif
    #ifdef COPROCESSOR_CP4
        reg1CPmask |= 3 << 8;
    #endif
    #ifdef COPROCESSOR_CP5
        reg1CPmask |= 3 << 10;
    #endif
    #ifdef COPROCESSOR_CP6
        reg1CPmask |= 3 << 12;
    #endif
    #ifdef COPROCESSOR_CP7
        reg1CPmask |= 3 << 14;
    #endif
    #ifdef COPROCESSOR_CP8
        reg1CPmask |= 3 << 16;
    #endif
    #ifdef COPROCESSOR_CP9
        reg1CPmask |= 3 << 18;
    #endif
    #ifdef COPROCESSOR_CP10
        reg1CPmask |= 3 << 20;
    #endif
    #ifdef COPROCESSOR_CP11
        reg1CPmask |= 3 << 22;
    #endif
    #ifdef COPROCESSOR_CP12
        reg1CPmask |= 3 << 24;
    #endif
    #ifdef COPROCESSOR_CP13
        reg1CPmask |= 3 << 26;
    #endif

    cp15_registers[CP15_REG1_SCB] = INVERT_W(CP15_REG1_SBZ_MASK) & (CP15_REG1_SBO_MASK | (ENDIANESS_BIGENDIAN << CP15_REG1_BPOS));
}

void cp15::executeOperation(Byte opcode, Byte rm, Byte rn, Byte rd, Byte info){
    switch(opcode){
        case 0:
            if(rd == 2) //Refresh PTE
                getPFN();
            break;
    }
}

void cp15::registerTransfer(Word *cpuReg, Byte opcode, Byte operand, Byte srcDest, Byte info, bool toCoproc){
    switch(srcDest){
        case 0: register0(cpuReg, opcode, operand, info, toCoproc); break;
        case 1: register1(cpuReg, info, toCoproc); break;
        case 2: register2(cpuReg, opcode, info, toCoproc); break;
        case 15: register15(cpuReg, opcode, toCoproc); break;
    }
}

void cp15::register0(Word *cpureg, Byte opcode, Byte operand, Byte info, bool toCoproc){
    if(opcode == 0 && !toCoproc){
        if(operand == 0){
            switch(info){
                case 0b001: *cpureg = CACHE_TYPE; break;
                case 0b010: *cpureg = TCM_TYPE; break;
                case 0b011: *cpureg = TLB_TYPE; break;
                case 0b100: *cpureg = MPU_TYPE; break;
                case 0b000:
                default:
                    *cpureg = MAIN_ID; break;
            }
        } else {
            *cpureg = bus->get_unpredictable();
        }
    }
}

void cp15::register1(Word *cpureg, Byte info, bool toCoproc){
    if(toCoproc){   //write
        switch(info){
            case 0b000: break;  //control register
            case 0b001: break;  //aux control register
            case 0b010:         //coprocessor access register
            cp15_registers[CP15_REG1_CCB] = *cpureg & reg1CPmask;
                break;
        }
    } else {        //read
        switch(info){
            case 0b000: *cpureg = cp15_registers[CP15_REG1_SCB]; break;
            case 0b001: *cpureg = 0; break;
            case 0b010: *cpureg = cp15_registers[CP15_REG1_CCB]; break;
        }
    }
}

void cp15::register2(Word *cpureg, Byte opcode, Byte info, bool toCoproc){
    switch(opcode){
        case 0: EntryHi(cpureg, info, toCoproc); break;
        case 1: EntryLo(cpureg, toCoproc); break;
    }
}

void cp15::register15(Word *cpureg, Byte opcode, bool toCoproc){
    if(toCoproc){   //write
        Word value = 0;
        Word mask = 0;
        switch(opcode){
            case 0: //Cause.ExcpCase
                mask = 0xFFFFFF;
                value = *cpureg & mask;
                break;
            case 1: //Cause.IP
                mask = 0xFF000000;
                value = *cpureg & mask;
                break;
        }
        cp15_registers[CP15_REG15_CAUSE] &= INVERT_W(mask);
        cp15_registers[CP15_REG15_CAUSE] |= value;
    } else {        //read
        *cpureg = cp15_registers[CP15_REG15_CAUSE];
    }
}

Word* cp15::getIPCauseRegister(){
    return cp15_registers + CP15_REG15_IP_CAUSE;
}

void cp15::EntryHi(Word *cpureg, Byte opcode, bool toCoproc){
    if(toCoproc){   //write
        bool changed = false;
        Word value = *cpureg;
        switch(opcode){
            case 0: //EntryHi.ASID
                value &= 0xFF0;
                if(value ^ (cp15_registers[CP15_REG2_EntryHi] & 0xFF0)){
                    cp15_registers[CP15_REG2_EntryHi] &= INVERT_W(0xFF0);
                    changed = true;
                }
                break;
            case 1: //EntryHi.VPN
                value &= 0xFFFFF000;
                if(value ^ (cp15_registers[CP15_REG2_EntryHi] & 0xFFFFF00)){
                    cp15_registers[CP15_REG2_EntryHi] &= INVERT_W(0xFFFFF00);
                    changed = true;
                }
                break;
        }
        if(changed){
            cp15_registers[CP15_REG2_EntryHi] |= value;
            getPFN();
        }
    } else {        //read
        *cpureg = cp15_registers[CP15_REG2_EntryHi];
    }
}

void cp15::EntryLo(Word *cpureg, bool toCoproc){
    if(!toCoproc){   //read
        *cpureg = cp15_registers[CP15_REG2_EntryLo];
    }
}

bool cp15::getPFN(){    //search Page Table for PTE and read EntryLo
    Word ASID = ((cp15_registers[CP15_REG2_EntryHi] >> 4) & 0xFF);
    Word VPF = cp15_registers[CP15_REG2_EntryHi] >> 12;
    bool shared = (cp15_registers[CP15_REG2_EntryHi] >= VM_SHSEG_START);
    Word pgTblPtr, address = ((2 * ASID) + (shared ? 4 : 0));
    AbortType abt = bus->readW(&address, &pgTblPtr);
    if(abt != ABT_NOABT){
        writeCause(abt);
        return false;
    }
    abt = bus->readW(&pgTblPtr, &pgTblPtr);
    if((pgTblPtr >> 24) != PAGE_TBL_MAGICNO){   // invalid page table header
        writeCause(PAGE_TBL_INVALID_H);
        return false;
    }
    Word pgTblSize = pgTblPtr & 0xFFFFF;
    /* scan page table */
    Word tmpHi;
    for(Word i = 0; i < pgTblSize; i++){
        address = pgTblPtr + (8 * (i+1));
        abt = bus->readW(&address, &tmpHi);
        if(abt != ABT_NOABT){
            writeCause(abt);
            return false;
        }
        if((tmpHi >> 12) == VPF){   // FOUND!
            address -= 4;
            bus->readW(&address, &cp15_registers[CP15_REG2_EntryLo]);
            writeCause(ABT_NOABT);
            return true;
        }
    }
    writeCause(PAGE_TBL_NOT_FOUND);
    return false;
}

void cp15::writeCause(Word causeW){
    register15(&causeW, 0, true);
}

#endif //UARM_CP15_CC
