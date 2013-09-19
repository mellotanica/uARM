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
    toolbar = new mainBar;
    display = new procDisplay(this);
    ramViewer = new ramView;

    centralLayout = new QVBoxLayout;

    centralLayout->addWidget(new QFLine(false));
    centralLayout->addWidget(toolbar);
    centralLayout->addWidget(new QFLine(false));
    centralLayout->addWidget(display);

    mainWidget->setLayout(centralLayout);

    clock = new QTimer(this);

    ramSize = MEM_SIZE_W;

    mac = new machine;

    connect(toolbar, SIGNAL(play(int)), this, SLOT(start(int)));
    connect(toolbar, SIGNAL(speedChanged(int)), this, SLOT(speedChanged(int)));
    connect(toolbar, SIGNAL(pause()), clock, SLOT(stop()));
    connect(toolbar, SIGNAL(reset()), this, SLOT(reset()));
    connect(toolbar, SIGNAL(open(QString)), this, SLOT(open(QString)));
    connect(toolbar, SIGNAL(showRam()), this, SLOT(showRam()));
    connect(toolbar, SIGNAL(step()), this, SLOT(step()));

    connect(clock, SIGNAL(timeout()), this, SLOT(step()));

    connect(this, SIGNAL(resetMachine(ulong)), this, SIGNAL(resetDisplay()));
    connect(this, SIGNAL(resetMachine(ulong)), mac, SLOT(reset(ulong)));
    connect(mac, SIGNAL(dataReady(Word*,Word*,Word*,QString)), display, SLOT(updateVals(Word*,Word*,Word*,QString)));
    connect(this, SIGNAL(resetDisplay()), display, SLOT(reset()));

    connect(this, SIGNAL(stop()), clock, SLOT(stop()));
    connect(this, SIGNAL(stop()), toolbar, SLOT(stop()));

    connect(mac, SIGNAL(updateStatus(QString)), toolbar, SLOT(updateStatus(QString)));

    reset();

    setCentralWidget(mainWidget);
}

void qarm::step(){
    if(!dataLoaded){
        QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                                  "Caution",
                                                                  "No program has been loaded.\nDo you want to start emulation anyways?",
                                                                  QMessageBox::Yes|QMessageBox::No);
        if(reply == QMessageBox::No){
            emit stop();
            return;
        } else {
            dataLoaded = true;
        }
    }
    mac->step();
}

void qarm::start(int speed){
    int time;
    if(speed <= IPSTRESH)
        time = 1000 / speed;
    else
        time = 1;
    clock->start(time);
}

void qarm::speedChanged(int speed){
    if(clock->isActive()){
        clock->stop();
        start(speed);
    }
}

void qarm::reset(){
    clock->stop();
    dataLoaded = false;
    emit resetMachine(ramSize);
}

void qarm::open(QString fname){
    QFile f (fname);
    if(!f.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Could not open file");
        return;
    }
    reset();
    QDataStream in(&f);
    ramMemory *ram = mac->getBus()->getRam();
    if(ram != NULL)
        fillMemory(ram, &in);
    f.close();
    dataLoaded = true;
    mac->refreshData();
}

void qarm::fillMemory(ramMemory *ram, QDataStream *in){
    Word tmp = 0, add;
    char read;
    Word address = PROG_START;
    int c = 0;
    while((in->readRawData(&read, 1)) == 1){
        add = (read & 0xFF) << (c*8);
        tmp |= add;
            if(c == 3){
                ram->writeW(&address, tmp, false);
                address += 4;
                tmp = 0;
            }
            c = (c+1)%4;
    }
    tmp = OP_HALT;
    ram->writeW(&address, tmp, false);
}

void qarm::showRam(){
    if(ramViewer->isHidden())
        ramViewer->show();
    else
        ramViewer->hide();
}
