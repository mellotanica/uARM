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
#include <QFileDialog>
#include <QFile>
#include "armProc/aout.h"

qarm::qarm(){
    ramSize = MEM_SIZE_W;
    mac = new machine;

    mainWidget = new QWidget;
    toolbar = new mainBar;
    display = new procDisplay(this);

    centralLayout = new QVBoxLayout;

    centralLayout->addWidget(new QFLine(false));
    centralLayout->addWidget(toolbar);
    centralLayout->addWidget(new QFLine(false));
    centralLayout->addWidget(display);

    mainWidget->setLayout(centralLayout);

    clock = new QTimer(this);

    connect(toolbar, SIGNAL(play(int)), this, SLOT(start(int)));
    connect(toolbar, SIGNAL(speedChanged(int)), this, SLOT(speedChanged(int)));
    connect(toolbar, SIGNAL(pause()), clock, SLOT(stop()));
    connect(toolbar, SIGNAL(reset()), this, SLOT(softReset()));
    connect(toolbar, SIGNAL(showRam()), this, SLOT(showRam()));
    connect(toolbar, SIGNAL(step()), this, SLOT(step()));
    connect(toolbar, SIGNAL(openRAM()), this, SLOT(selectCore()));
    connect(toolbar, SIGNAL(openBIOS()), this, SLOT(selectBios()));

    connect(clock, SIGNAL(timeout()), this, SLOT(step()));

    connect(this, SIGNAL(resetMachine(ulong)), mac, SLOT(reset(ulong)));
    connect(this, SIGNAL(resetMachine(ulong)), this, SIGNAL(resetDisplay()));
    connect(mac, SIGNAL(dataReady(Word*,Word*,Word*,QString)), display, SLOT(updateVals(Word*,Word*,Word*,QString)));
    connect(this, SIGNAL(resetDisplay()), display, SLOT(reset()));

    connect(this, SIGNAL(stop()), clock, SLOT(stop()));
    connect(this, SIGNAL(stop()), toolbar, SLOT(stop()));

    connect(mac, SIGNAL(updateStatus(QString)), toolbar, SLOT(updateStatus(QString)));

    setCentralWidget(mainWidget);
}

void qarm::softReset(){
    clock->stop();
    initialized = false;
    emit resetMachine(ramSize);
    doReset = false;
    if(dataLoaded && biosLoaded)
        initialize();
}

void qarm::initialize(){
    openBIOS();
    openRAM();
    initialized = true;
}

void qarm::step(){
    if(!(dataLoaded && biosLoaded)){
        QString msg = "";
        int c = 0;
        if(!biosLoaded){
            if(c > 0)
                msg += ", ";
            msg += "BIOS ROM";
            c++;
        }
        if(!dataLoaded){
            if(c > 0)
                msg += " and ";
            msg += "Program File";
            c++;
        }
        if(c > 1)
            msg += " were";
        else
            msg += " was";
        msg += " not selected.\nDo you want to start emulation anyways?";

        QMessageBox::StandardButton reply = QMessageBox::question(this,"Caution", msg, QMessageBox::Yes|QMessageBox::No);

        if(reply == QMessageBox::No){
            emit stop();
            return;
        } else {
            dataLoaded = biosLoaded = true;
        }
    }
    if(doReset){
        emit resetMachine(ramSize);
        doReset = false;
    }
    if(!initialized){
        initialize();
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

void qarm::showRam(){
    if(initialized){
        ramView *ramWindow = new ramView(mac, this);
        connect(this, SIGNAL(resetMachine(ulong)), ramWindow, SLOT(update()));
        connect(mac, SIGNAL(dataReady(Word*,Word*,Word*,QString)), ramWindow, SLOT(update()));
        ramWindow->show();
    } else {
        QMessageBox::warning(this, "Warning", "Machine not initialized,\ncannot display memory contents.", QMessageBox::Ok);
    }
}

void qarm::selectCore(){
    if(dataLoaded){
        QMessageBox::StandardButton reply = QMessageBox::question(this,"Caution",
                                                                  "Program File already loaded..\nReset machine and load new Program?",
                                                                  QMessageBox::Yes|QMessageBox::No);
        if(reply == QMessageBox::No)
            return;
        else {
            initialized = false;
            doReset = true;
        }
    }
    QString fileName = QFileDialog::getOpenFileName(this, "Open Program File", "", "Program Files (*.core.uarm);;Binary Files (*.bin);;All Files (*.*)");
    if(fileName != ""){
        coreF = fileName;
        dataLoaded = true;
    }
}

void qarm::selectBios(){
    if(biosLoaded){
        QMessageBox::StandardButton reply = QMessageBox::question(this,"Caution",
                                                                  "BIOS ROM already loaded..\nReset machine and load new BIOS?",
                                                                  QMessageBox::Yes|QMessageBox::No);
        if(reply == QMessageBox::No)
            return;
        else {
            initialized = false;
            doReset = true;
        }
    }
    QString fileName = QFileDialog::getOpenFileName(this, "Open BIOS File", "", "BIOS Files (*.rom.uarm);;Binary Files (*.bin);;All Files (*.*)");
    if(fileName != ""){
        biosF = fileName;
        biosLoaded = true;
    }
}

void qarm::openRAM(){
    if(coreF != ""){
        QFile f (coreF);
        if(!f.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, "Error", "Could not open file");
            dataLoaded = false;
            return;
        }
        QDataStream in(&f);
        ramMemory *ram = mac->getBus()->getRam();
        if(ram != NULL){
            Word len = (Word) f.size();
            char *buffer = new char[len];
            int sz = in.readRawData(buffer, len);
            if(sz <= 0 || (buffer[0] | buffer[1]<<8 | buffer[2]<<16 | buffer[3]<<24) != COREFILEID){
                QMessageBox::critical(this, "Error", "Irregular core file");
                dataLoaded = false;
                return;
            }
            sz -= 4;
            if(sz <= 0 || !mac->getBus()->loadRAM(buffer+4, (Word) sz, true)){
                QMessageBox::critical(this, "Error", "Problems while loading RAM");
                dataLoaded = false;
                return;
            }
            delete [] buffer;
        }
        f.close();
        mac->refreshData();
    }
}

void qarm::openBIOS(){
    if(biosF != ""){
        QFile f (biosF);
        if(!f.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, "Error", "Could not open file");
            biosLoaded = false;
            return;
        }
        QDataStream in(&f);
        Word len = (Word) f.size();
        char *buffer = new char[len];
        Word sz = in.readRawData(buffer, len);
        if(sz <= 0 || (buffer[0] | buffer[1]<<8 | buffer[2]<<16 | buffer[3]<<24) != BIOSFILEID){
            QMessageBox::critical(this, "Error", "Irregular BIOS file");
            biosLoaded = false;
            return;
        }
        sz -= 8;
        if(sz <= 0 || !mac->getBus()->loadBIOS(buffer+8, (Word) sz)){
            QMessageBox::critical(this, "Error", "Problems while flashing BIOS ROM");
            biosLoaded = false;
            return;
        }
        delete [] buffer;
        f.close();
    }
}

MachineConfig *getMachineConfig(){
    return machineConfigs;
}
