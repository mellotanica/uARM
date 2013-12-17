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

#include "armProc/machine.h"
#include "armProc/processor.h"
//#include "armProc/coprocessor_interface.h"

machine::machine(QObject *parent) : QObject(parent){
    sysbus = new systemBus(this);
}

machine::~machine(){
    if(sysbus != NULL)
        delete sysbus;
}

void machine::initMac(){
}

void machine::reset(){
    sysbus->reset();
    emit updateStatus(status2QString());
}

void machine::step(){
    processor *cpu = sysbus->getProcessor(0);
    if(cpu->getStatus() != PS_HALTED){
        if(cpu->branchHappened()){
            cpu->prefetch();
        } else {
            cpu->nextCycle();
        }
    }
    if(cpu->getStatus() != PS_HALTED){
        cpu->cycle();
        refreshData();
    }
}

void machine::refreshData(){
    if(!fullUIupdate){
        if(ticksFromUpdate < UI_UPDATE_THRESHOLD){
            ticksFromUpdate++;
            return;
        } else {
            ticksFromUpdate = 0;
        }
    }
    processor *cpu = sysbus->getProcessor(0);
    emit updateStatus(status2QString());
    QString mnem = QString::fromStdString(cpu->mnemonicOPcode) + (cpu->isOPcodeARM ? "(ARM)" : "(Thumb)" );
    emit dataReady(cpu->getRegList(), cpu->getCP15()->getRegList() , sysbus->pipeline, sysbus->getToDHI(), sysbus->getToDLO(), sysbus->getTimer(), mnem);

}

QString machine::status2QString(){
    switch(sysbus->getProcessor(0)->getStatus()){
        case PS_HALTED: return "HALTED"; break;
        case PS_IDLE: return "IDLE"; break;
        case PS_RUNNING: return "RUNNING"; break;
        case PS_RESET: return "RESET"; break;
    }
    return "UNKNOWN";
}

void machine::run(){
    while(sysbus->getProcessor(0)->getStatus() != PS_HALTED)
		step();
}

#endif //UARM_MACHINE_CC
