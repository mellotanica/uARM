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

void qarm::softReset(){
    reset();
    if(dataLoaded && biosLoaded)
        initialize();
}

void qarm::reset(){
    clock->stop();
    initialized = false;
    emit resetMachine(ramSize);
}

void qarm::showRam(){
    ramView *ramWindow = new ramView(mac, this);
    connect(this, SIGNAL(resetMachine(ulong)), ramWindow, SLOT(update()));
    connect(mac, SIGNAL(dataReady(Word*,Word*,Word*,QString)), ramWindow, SLOT(update()));
    ramWindow->show();
}

void qarm::initialize(){
    openBIOS();
    openRAM();
    initialized = true;
}

void qarm::selectCore(){
    if(dataLoaded){
        QMessageBox::StandardButton reply = QMessageBox::question(this,"Caution",
                                                                  "Program File already loaded..\nReset machine and load new Program?",
                                                                  QMessageBox::Yes|QMessageBox::No);
        if(reply == QMessageBox::No)
            return;
        else
            reset();
    }
    QString fileName = QFileDialog::getOpenFileName(this, "Open Program File", "", "Binary Files (*.bin);;All Files (*.*)");
    if(fileName != ""){
        QFile f (fileName);
        if(!f.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, "Error", "Could not open file");
            return;
        }
        f.close();
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
        else
            reset();
    }
    QString fileName = QFileDialog::getOpenFileName(this, "Open BIOS File", "", "Binary Files (*.bin);;All Files (*.*)");
    if(fileName != ""){
        QFile f (fileName);
        if(!f.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, "Error", "Could not open file");
            return;
        }
        f.close();
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
            in.readRawData(buffer, len);
            if(!mac->getBus()->loadRAM(buffer, len, true)){
                QMessageBox::critical(this, "Error", "Problems while loading RAM");
                dataLoaded = false;
                return;
            }
            delete buffer;
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
        in.readRawData(buffer, len);
        if(!mac->getBus()->loadBIOS(buffer, len)){
            QMessageBox::critical(this, "Error", "Problems while flashing BIOS ROM");
            biosLoaded = false;
            return;
        }
        delete buffer;
        f.close();
    }
}
