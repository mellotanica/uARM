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
 * This class does machine initialization and component binding
 */

#ifndef UARM_MACHINE_H
#define UARM_MACHINE_H

#include "services/util.h"
#include "const.h"
#include "bus.h"
#include "pu.h"
#include "cp15.h"
#include "coprocessor_interface.h"
#include "processor.h"

class machine{
public:
	machine(Word ramSize);
	~machine();
	
	processor *getCPU() {return cpu;};
	systemBus *getBus() {return sysbus;};
	
	void init();
	void step();
	void run();
	
private:
	systemBus *sysbus;
	processor *cpu;
};

#endif //UARM_MACHINE_CC
