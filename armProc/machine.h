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
#include "armProc/const.h"
#include "armProc/bus.h"
#include "armProc/stoppoint.h"
#include <QObject>

enum StopCause {
    SC_USER         = 1 << 0,
    SC_BREAKPOINT   = 1 << 1,
    SC_SUSPECT      = 1 << 2,
    SC_EXCEPTION    = 1 << 3,
    SC_UTLB_KERNEL  = 1 << 4,
    SC_UTLB_USER    = 1 << 5
};

class machine : public QObject{
    Q_OBJECT
public:
    machine(StoppointSet* breakpoints, StoppointSet* suspects, StoppointSet* tracepoints);
	~machine();

    //if refRate equals 0 the ui is updated at each instruction, else refRate specifies the amount of instructions to skip
    void setUIupdate(unsigned int refRate){refreshRate = refRate; fullUIupdate = !refRate;}

    systemBus *getBus() {return sysbus;}

    unsigned int getStopCause() const {return stopCause;}
    unsigned int getActiveBreakpoint() const {return breakpointId;}
    unsigned int getActiveSuspect() const {return suspectId;}

    void setStopMask(unsigned int mask) {stopMask = mask;}
    unsigned int getStopMask() const {return stopMask;}

    bool isStopRequested() const {return stopRequested;}

    void updateTLB(unsigned int index);

    void HandleBusAccess(Word pAddr, Word access, processor* cpu);
    void HandleVMAccess(Word asid, Word vaddr, Word access, processor* cpu);

signals:
    void dataReady(Word *cpu, Word *cp15, Word *pipeline, Word todH, Word todL, Word timer, QString mnemonic);
    void updateStatus(QString state);
    void tlbChanged(unsigned int index);

public slots:
    void step();
    void refreshData();
    void refreshData(bool force);
    void clearCause();

private slots:
    void run();
    void reset();
    void toggleBP(bool status);
    void toggleTLBPause(bool status);

private:
    bool breakpointStatus = true;
    bool stopOnTLB = false;
    bool fullUIupdate = true;
    unsigned int refreshRate = 1;
    unsigned int ticksFromUpdate = 0;
    bool idleNotified = false;
    bool stopRequested = false;

    QString status2QString();

    systemBus *sysbus;

    unsigned int stopMask;
    //STATIC: only one processor to track..
    unsigned int stopCause;
    unsigned int breakpointId;
    unsigned int suspectId;

    StoppointSet* breakpoints;
    StoppointSet* suspects;
    StoppointSet* tracepoints;
};

#endif //UARM_MACHINE_CC
