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

#include "qarm.h"

qarm::qarm(){
    mainWidget = new QWidget;
    toolbar = new mainBar(mainWidget);
    display = new procDisplay(mainWidget);

    resetFlag = true;

    layout = new QVBoxLayout;
    layout->addWidget(new QFLine(false));
    layout->addWidget(toolbar);
    layout->addWidget(new QFLine(false));
    layout->addWidget(display);

    mainWidget->setLayout(layout);

    clock = new QTimer(this);

    connect(toolbar, SIGNAL(play(int)), this, SLOT(start(int)));
    connect(toolbar, SIGNAL(speedChanged(int)), this, SLOT(start(int)));
    connect(toolbar, SIGNAL(pause()), clock, SLOT(stop()));
    connect(toolbar, SIGNAL(stop()), this, SLOT(stop()));
    connect(toolbar, SIGNAL(open(QString)), this, SLOT(open(QString)));
    connect(toolbar, SIGNAL(showRam()), this, SLOT(showRam()));
    connect(toolbar, SIGNAL(step()), this, SLOT(step()));

    connect(clock, SIGNAL(timeout()), this, SLOT(step()));

    connect(mac, SIGNAL(dataReady(Word*,Word*,Word*,QString)), display, SLOT(updateVals(Word*,Word*,Word*,QString)));
    connect(this, SIGNAL(resetDisplay()), display, SLOT(reset()));

    ramSize = MEM_SIZE_W;

    setCentralWidget(mainWidget);
}

void qarm::start(int speed){

}

void qarm::step(){
    if(resetFlag){
        emit resetDisplay();
        if(mac != NULL)
            delete mac;
        mac = new machine(ramSize);
    }
    mac->step();

}

void qarm::stop(){
    resetFlag = true;
}

void qarm::open(QString fname){

}

void qarm::showRam(){

}
