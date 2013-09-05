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

#ifndef UARM_MACHINE_CC
#define UARM_MACHINE_CC

#include "services/util.h"
#include "bus.h"
#include "pu.h"
#include "coprocessor_interface.h"
#include "processor.h"
#include "machine.h"

systemBus *pu::bus;
util *util::instance;

machine::machine(Word ramSize){
	cpu = new processor();
	
	sysbus = cpu->getBus();
	
	sysbus->getRam()->init(ramSize);
	
	*(cpu->getPC()) = PROG_START;
}

machine::~machine(){
	delete sysbus;
}

void machine::init(){
	for(int i = 0; i < 2; i++)
		cpu->nextCycle();
}

void machine::step(){
	if(sysbus->branchHappened){
		cpu->prefetch();
		sysbus->branchHappened = false;
	} else
		cpu->nextCycle();
	cpu->cycle();
}

void machine::run(){
	while(cpu->getStatus() != PS_HALTED)
		step();
}

#endif //UARM_MACHINE_CC
