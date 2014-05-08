/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uMPS - A general purpose computer system simulator
 *
 * Copyright (C) 2010 Tomislav Jonjic
 * Copyright (C) 2014 Marco Melletti
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

#ifndef QMPS_DEBUG_SESSION_H
#define QMPS_DEBUG_SESSION_H

#include <QObject>
#include <QApplication>
#include <QMainWindow>

#include "services/lang.h"
#include "services/symbol_table.h"
#include "armProc/stoppoint.h"
//STATIC: only breakpoints are available right now..
/*#include "armProc/machine.h"
#include "qmps/cpu_status_map.h"
#include "qarm/stoppoint_list_model.h"

enum MachineStatus {
    MS_HALTED,
    MS_RUNNING,
    MS_STOPPED
};

class QAction;
class Machine;
class QTimer;
*/
class DebugSession : public QObject {
    Q_OBJECT

//    Q_PROPERTY(int speed READ getSpeed WRITE setSpeed NOTIFY SpeedChanged)

public:

/*
    static const int kNumSpeedLevels = 5;
    static const int kMaxSpeed = kNumSpeedLevels - 1;

    static const int kDefaultStopMask = (SC_BREAKPOINT |
                                         SC_SUSPECT |
                                         SC_EXCEPTION);
*/
    DebugSession(QApplication *application, QMainWindow *mainWindow);
/*
    MachineStatus getStatus() const { return status; }

    bool isStopped() const { return status == MS_STOPPED; }
    bool isStoppedByUser() const { return stoppedByUser; }
    bool isRunning() const { return status == MS_RUNNING; }
    bool isStarted() const { return status != MS_HALTED; }

    void halt();

    unsigned int getStopMask() const { return stopMask; }
    int getSpeed() const { return speed; }

    Machine* getMachine() const { return machine.get(); }*/
    SymbolTable* getSymbolTable() { return symbolTable.get(); }

    StoppointSet* getBreakpoints() { return &breakpoints; }

    StoppointSet* getSuspects() { return &suspects; }
    StoppointSet* getTracepoints() { return &tracepoints; }
/*
    StoppointListModel* getBreakpointListModel() { return bplModel.get(); }

    const CpuStatusMap* getCpuStatusMap() const { return cpuStatusMap.get(); }

    // Global actions
    QAction* startMachineAction;
    QAction* haltMachineAction;
    QAction* resetMachineAction;

    QAction* debugContinueAction;
    QAction* debugStepAction;
    QAction* debugStopAction;

*/
public Q_SLOTS:
    void resetSymbolTable();
/*
    void setStopMask(unsigned int value);
    void setSpeed(int value);
    void stop();

*/
Q_SIGNALS:
    void stabUpdated();
    void MachineRan();
    void stabUnavavilable();

/*
    void StatusChanged();
    void MachineStarted();
    void MachineStopped();
    void MachineHalted();
    void MachineReset();
    void DebugIterationCompleted();

    void SpeedChanged(int);
*/
private:
/*
    static const uint32_t kMaxSkipped = 50000;

    void createActions();
    void setStatus(MachineStatus newStatus);

    void initializeMachine();

    void step(unsigned int steps);
    void runStepIteration();
    void runContIteration();

    MachineStatus status;
    scoped_ptr<Machine> machine;
*/
    void relocateStoppoints(const SymbolTable* newTable, StoppointSet* set);

    QMainWindow *mainW;
    QApplication *app;
    scoped_ptr<SymbolTable> symbolTable;
/*
    // We need a "proxy" stop mask here since it has to live through
    // machine reconfigurations, resets, etc.
    unsigned int stopMask;

    unsigned int speed;
    static const unsigned int kIterCycles[kNumSpeedLevels];
    static const unsigned int kIterInterval[kNumSpeedLevels];

    scoped_ptr<StoppointListModel> bplModel;
*/
    StoppointSet breakpoints;
    StoppointSet suspects;
    StoppointSet tracepoints;
/*
    scoped_ptr<CpuStatusMap> cpuStatusMap;

    bool stoppedByUser;

    bool stepping;
    unsigned int stepsLeft;

    QTimer* timer;
    QTimer* idleTimer;

    uint32_t idleSteps;
*/
private Q_SLOTS:

    void onMachineConfigChanged();
/*
    void startMachine();
    void onHaltMachine();
    void onResetMachine();
    void onContinue();
    void onStep();

    void updateActionSensitivity();

    void runIteration();
    void skip();
*/
};


class DebuggerHolder {
private:
    static DebuggerHolder *instance;
    static DebugSession *session;

    DebuggerHolder() {}

public:
    ~DebuggerHolder() {}

    static DebuggerHolder *getInstance();

    DebugSession *getDebugSession() {return session;}
    void setDebugSession(DebugSession *debug) {session = debug;}
};

#endif // QMPS_DEBUG_SESSION_H
