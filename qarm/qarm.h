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

#ifndef QARM_QARM_H
#define QARM_QARM_H

#include "armProc/machine.h"
#include "qarm/mainbar.h"
#include "services/debug_session.h"
#include "qarm/procdisplay.h"
#include "qarm/guiConst.h"
#include "qarm/ramview.h"
#include "qarm/QLine.h"
#include "qarm/breakpoint_window.h"
#include "qarm/structures_window.h"
#include "qarm/tlb_window.h"
#include "terminal_window.h"
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QTimer>
#include <QFile>
#include <QDataStream>
#include <QPointer>
#include <QApplication>
#include <QShortcut>

class qarm : public QMainWindow{
    Q_OBJECT
public:
    qarm(QApplication *app, QFile *confFile, bool autorun, bool runandexit, bool gui);
    machine *getMachine() {return mac;}
    MachineConfig *getMachineConfig() {return MC_Holder::getInstance()->getConfig();}
    virtual void show();
    void disableMainbar(bool fromConfig, bool value);

protected:
    virtual void closeEvent(QCloseEvent* event);

signals:
    void resetDisplay();
    void resetMachine();
    void stopSig();
    void setTerminalEnabled(unsigned int devNo, bool enabled);
    void resumeExec();
    void poweredOn();
    void poweredOff();

public slots:
    void stop();
    void kill(int exitcode);

private slots:
    void start(int speed);
    void step();
    void pause();
    void speedChanged(int speed);
    void softReset();
    void selectCore();
    void selectBios();
    void showRam();
    void showRamSel(Word start, Word end, QString label, bool offset);
    void showConfigDialog();
    void showTerminal(unsigned int devNo);
    void onMachineHalted();
    void powerOn();
    void powerOff();
    void closeFWindow();
    void onMachineReset();

private:
    QApplication *application;
    machine *mac;
    bool dataLoaded = false, biosLoaded = false, initialized = false, doReset = true, resuming = true;
    std::string coreF, biosF;

    breakpoint_window *bpWindow;
    structures_window *structWindow;

    tlb_window *tlbWindow;

    QShortcut *closeSc;

    QWidget *mainWidget;
    procDisplay *display;
    QVBoxLayout *centralLayout;
    mainBar *toolbar;
    QTimer *clock;

    QPointer<TerminalWindow> terminalWindows[N_DEV_PER_IL];

    bool initialize();
    bool openRAM();
    bool openBIOS();
    QWidget *createConfigTab();
};

#endif // QARM_QARM_H
