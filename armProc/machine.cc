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
#include "services/debug.h"
//#include "armProc/coprocessor_interface.h"

#include <QString>

machine::machine(StoppointSet* breakpoints,
                 StoppointSet* suspects,
                 StoppointSet* tracepoints)
    : breakpoints(breakpoints),
      suspects(suspects),
      tracepoints(tracepoints)
{
    sysbus = new systemBus(this);
    clearCause();
}

machine::~machine(){
    if(sysbus != NULL)
        delete sysbus;
}

void machine::reset(){
    sysbus->reset();
    emit updateStatus(status2QString());
}

void machine::step(){
    processor *cpu = sysbus->getProcessor(0);
    stopRequested = false;
    if(cpu->getStatus() != PS_HALTED){
        sysbus->ClockTick();
        cpu->clockTick();

        if(cpu->getStatus() != PS_IDLE)
            cpu->cycle();

        refreshData();
    }
}

void machine::refreshData(){
    refreshData(false);
}

void machine::refreshData(bool force){
    processor *cpu;
    if(!fullUIupdate && !force){
        if(ticksFromUpdate < refreshRate){
            ticksFromUpdate++;
            return;
        } else {
            ticksFromUpdate = 0;
        }
        cpu = sysbus->getProcessor(0);
        if(cpu->getStatus() == PS_IDLE){
            if(idleNotified)
                return;
            else
                idleNotified = true;
        } else {
            idleNotified = false;
        }
    }
    else
        cpu = sysbus->getProcessor(0);
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

void machine::HandleBusAccess(Word pAddr, Word access, processor* cpu)
{
    // Check for breakpoints and suspects
    switch (access) {
    case READ:
    case WRITE:
        if (stopMask & SC_SUSPECT) {
            Stoppoint* suspect = suspects->Probe(MAXASID, pAddr,
                                                 (access == READ) ? AM_READ : AM_WRITE,
                                                 cpu);
            if (suspect != NULL) {
                stopCause |= SC_SUSPECT;
                suspectId = suspect->getId();
                stopRequested = true;
            }
        }
        break;

    case EXEC:
        if (breakpointStatus) {
            //STATIC: revert to maxasid or proper value when VM will be effective..
            Stoppoint* breakpoint = breakpoints->Probe(MC_Holder::getInstance()->getConfig()->getSymbolTableASID(), pAddr, AM_EXEC, cpu);
            if (breakpoint != NULL) {
                stopCause |= SC_BREAKPOINT;
                breakpointId = breakpoint->getId();
                stopRequested = true;
            }
        }
        break;

    default:
        AssertNotReached();
    }

    // Check for traced ranges
    if (access == WRITE) {
        Stoppoint* tracepoint = tracepoints->Probe(MAXASID, pAddr, AM_WRITE, cpu);
        (void) tracepoint;
    }
}

void machine::HandleVMAccess(Word asid, Word vaddr, Word access, processor* cpu)
{
    switch (access) {
    case READ:
    case WRITE:
        if (stopMask & SC_SUSPECT) {
            Stoppoint* suspect = suspects->Probe(asid, vaddr,
                                                 (access == READ) ? AM_READ : AM_WRITE,
                                                 cpu);
            if (suspect != NULL) {
                stopCause |= SC_SUSPECT;
                suspectId = suspect->getId();
                stopRequested = true;
            }
        }
        break;

    case EXEC:
        if (breakpointStatus) {
            Stoppoint* breakpoint = breakpoints->Probe(asid, vaddr, AM_EXEC, cpu);
            if (breakpoint != NULL) {
                stopCause |= SC_BREAKPOINT;
                breakpointId = breakpoint->getId();
                stopRequested = true;
            }
        }
        break;

    default:
        AssertNotReached();
    }
}

void machine::toggleBP(bool status){
    breakpointStatus = status;
}

void machine::clearCause(){
    stopCause = 0;
}

void machine::updateTLB(unsigned int index){
    emit tlbChanged(index);
}

#endif //UARM_MACHINE_CC
