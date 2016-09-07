/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uARM
 *
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

#include "structures_window.h"

#include <QCloseEvent>

#include "qarm/QLine.h"
#include "services/debug_session.h"

structures_window::structures_window(machine *mac, QWidget * parent, Qt::WindowFlags flags):
    QMainWindow(parent, flags),
    mac(mac)
{
    mainWidget = new QWidget();
    mainLayout = new QVBoxLayout(mainWidget);

    selectWidget = new SelectStructuresDialog(mainWidget);

    ramViewer = new HexView(0, 0, mac, mainWidget);
    initRamViewer();

    connect(selectWidget, SIGNAL(selectedObject(Word, Word)), ramViewer, SLOT(MoveInterval(Word,Word)));
    connect(selectWidget, SIGNAL(openRam(Word,Word,QString,bool)), this, SIGNAL(openRamViewer(Word,Word,QString,bool)));
    connect(ramViewer, SIGNAL(doubleClicked(Word)), selectWidget, SLOT(triggerOpenRam(Word)));

    showRam = new QToolButton(mainWidget);
    showRam->setText("Show in Bus Inspector");
    showRam->setAccessibleName(showRam->text());
    showRam->setAccessibleDescription("Show selected data structures in separate a Bus Inspector window");
    connect(showRam, SIGNAL(clicked()), selectWidget, SLOT(triggerOpenRam()));

    mainLayout->addWidget(selectWidget);
    mainLayout->addWidget(showRam);
    mainLayout->addWidget(ramViewer);

    mainWidget->setLayout(mainLayout);

    hide();

    connect(DebuggerHolder::getInstance()->getDebugSession(), SIGNAL(stabUpdated()), this, SLOT(updateContent()));
    connect(DebuggerHolder::getInstance()->getDebugSession(), SIGNAL(stabUnavavilable()), this, SLOT(updateContent()));

    setCentralWidget(mainWidget);
    setWindowTitle("Structures");
}

structures_window::~structures_window(){

}

void structures_window::initRamViewer(){
    ramViewer->MoveInterval(0, 0x1c);
}

void structures_window::closeEvent(QCloseEvent *event){
    emit hiding();
    hide();
    event->ignore();
}

void structures_window::updateContent(){
    selectWidget->updateContent();
    update();
}

void structures_window::update(){
    if(!selectWidget->isStabLoaded()){
        initRamViewer();
    }
    ramViewer->Refresh();
}
