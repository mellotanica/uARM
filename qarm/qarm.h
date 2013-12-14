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
#include "qarm/procdisplay.h"
#include "qarm/guiConst.h"
#include "qarm/ramview.h"
#include "qarm/QLine.h"
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QTimer>
#include <QFile>
#include <QDataStream>
#include <QMessageBox>
#include <QApplication>

class qarm : public QMainWindow{
    Q_OBJECT
public:
    qarm(QApplication *app);
    machine *getMachine() {return mac;}

signals:
    void resetDisplay();
    void resetMachine();
    void stop();

private slots:
    void start(int speed);
    void step();
    void speedChanged(int speed);
    void softReset();
    void selectCore();
    void selectBios();
    void showRam();
    void showConfigDialog();

private:
    QApplication *application;
    machine *mac;
    bool dataLoaded = false, biosLoaded = false, initialized = false, doReset = true;
    QString coreF, biosF;

    QWidget *mainWidget;
    procDisplay *display;
    QVBoxLayout *centralLayout;
    mainBar *toolbar;
    QTimer *clock;

    bool initialize();
    bool openRAM();
    bool openBIOS();
    QWidget *createConfigTab();
};

#endif // QARM_QARM_H
