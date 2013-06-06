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

#include "const.h"
#include "pu.h"

class cp15 : public coprocessor{
public:
	cp15();
	~cp15() {};
	
	Word *getRegister(unsigned int i) {return &cp15_registers[i];};
	
private:
	ramMemory *ram;
	
	Word cp15_registers[CP15_REGISTERS_NUM];
	
	void execute();
};

#endif //UARM_CP15_CC
