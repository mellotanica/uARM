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
 
/* 
 * Interface common to pocessing units (main processor and coprocessors)
 * Provides the common execution structure
 */

#ifndef UARM_PU_H
#define UARM_PU_H

#include "armProc/const.h"
//LOOP: pu
#include "armProc/bus.h"

class pu{
public:
	pu() {
		if(bus == NULL)
			bus = new systemBus();
    }
    virtual ~pu() {
        if(bus != NULL){
            delete bus;
            bus = NULL;
        }
    }
	
    virtual Word *getRegList() = 0;

protected:
	static systemBus *bus;
};

class coprocessor : public pu{
public:
    coprocessor() : pu() {}
    ~coprocessor(){}
	
    virtual void reset() = 0;

    virtual Word *getRegList() = 0;

	virtual Word *getRegister(Word regNum) = 0;
	
	virtual void executeOperation(Byte opcode, Byte rm, Byte rn, Byte rd, Byte info) = 0;
    virtual void registerTransfer(Word *cpuReg, Byte opcode, Byte operand, Byte srcDest, Byte info, bool toCoproc) = 0;
};

#endif //UARM_PU_H
