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
#include <QObject>


class machine : public QObject{
    Q_OBJECT
public:
    machine(QObject *parent = 0);
	~machine();

    //if refRate equals 0 the ui is updated at each instruction, else refRate specifies the amount of instructions to skip
    void setUIupdate(unsigned int refRate){refreshRate = refRate; fullUIupdate = !refRate;}

    systemBus *getBus() {return sysbus;}

signals:
    void dataReady(Word *cpu, Word *cp15, Word *pipeline, Word todH, Word todL, Word timer, QString mnemonic);
    void updateStatus(QString state);

public slots:
    void step();
    void refreshData();

private slots:
    void run();
    void reset();

private:
    bool fullUIupdate = true;
    unsigned int refreshRate = 1;
    unsigned int ticksFromUpdate = 0;

    QString status2QString();

    systemBus *sysbus;
};

#endif //UARM_MACHINE_CC
