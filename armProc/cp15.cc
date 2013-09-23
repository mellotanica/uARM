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

cp15::cp15(): coprocessor() {
	ram = bus->getRam();
    for(int i = 0; i < CP15_REGISTERS_NUM; i++)
        cp15_registers[i] = 0;
    reg1CPmask = 0;
    #ifdef COPROCESSOR_CP0
        mask |= 3;
    #endif
    #ifdef COPROCESSOR_CP1
        mask |= 3 << 2;
    #endif
    #ifdef COPROCESSOR_CP2
        mask |= 3 << 4;
    #endif
    #ifdef COPROCESSOR_CP3
        mask |= 3 << 6;
    #endif
    #ifdef COPROCESSOR_CP4
        mask |= 3 << 8;
    #endif
    #ifdef COPROCESSOR_CP5
        mask |= 3 << 10;
    #endif
    #ifdef COPROCESSOR_CP6
        mask |= 3 << 12;
    #endif
    #ifdef COPROCESSOR_CP7
        mask |= 3 << 14;
    #endif
    #ifdef COPROCESSOR_CP8
        mask |= 3 << 16;
    #endif
    #ifdef COPROCESSOR_CP9
        mask |= 3 << 18;
    #endif
    #ifdef COPROCESSOR_CP10
        mask |= 3 << 20;
    #endif
    #ifdef COPROCESSOR_CP11
        mask |= 3 << 22;
    #endif
    #ifdef COPROCESSOR_CP12
        mask |= 3 << 24;
    #endif
    #ifdef COPROCESSOR_CP13
        mask |= 3 << 26;
    #endif
}

void cp15::executeOperation(Byte opcode, Byte rm, Byte rn, Byte rd, Byte info){
	cp15_registers[4] = 0x44556644;
}

void cp15::registerTransfer(Word *cpuReg, Byte opcode, Byte operand, Byte srcDest, Byte info, bool toCoproc){
    switch(srcDest){
        case 0: register0(cpuReg, opcode, operand, info, toCoproc); break;
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

void cp15::register1(Word *cpureg, Byte opcode, Byte operand, Byte info, bool toCoproc){
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

#endif //UARM_CP15_CC
