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

machine::machine(QObject *parent) : QObject(parent){
    initMac();
}

machine::machine(Word ramSize, QObject *parent) : QObject(parent){
    initMac();
    sysbus->getRam()->init(ramSize);
}

machine::~machine(){
	delete sysbus;
}

void machine::initMac(){
    cpu = new processor();

    sysbus = cpu->getBus();

    *(cpu->getPC()) = PROG_START;

    sysbus->branchHappened = true;
}

void machine::reset(unsigned long memSize){
    delete cpu;
    initMac();
    sysbus->getRam()->init(memSize);
}

void machine::step(){
	if(sysbus->branchHappened){
		cpu->prefetch();
		sysbus->branchHappened = false;
	} else
		cpu->nextCycle();
	cpu->cycle();
    QString mnem = QString::fromStdString(cpu->mnemonicOPcode) + (cpu->isOPcodeARM ? "(ARM)" : "(Thumb)" );
    emit dataReady(cpu->getRegList(), cpu->getCopInt()->getCoprocessor(15)->getRegList() , sysbus->pipeline, mnem);
}

void machine::run(){
	while(cpu->getStatus() != PS_HALTED)
		step();
}

#endif //UARM_MACHINE_CC
