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
    connect(selectWidget, SIGNAL(selectedObject(Word, Word)), this, SLOT(onSelectedObject(Word, Word)));

    ramViewer = new HexView(0, 0x1c, mac, mainWidget);

    mainLayout->addWidget(selectWidget);
    mainLayout->addWidget(ramViewer);

    mainWidget->setLayout(mainLayout);

    hide();

    connect(DebuggerHolder::getInstance()->getDebugSession(), SIGNAL(stabUpdated()), this, SLOT(symtabReady()));
    connect(DebuggerHolder::getInstance()->getDebugSession(), SIGNAL(stabUnavavilable()), this, SLOT(symtabMissing()));

    setCentralWidget(mainWidget);
    setWindowTitle("Structures");
}

structures_window::~structures_window(){

}

void structures_window::closeEvent(QCloseEvent *event){
    emit hiding();
    hide();
    event->ignore();
}

void structures_window::updateContent(){
    selectWidget->updateContent();
    if(!selectWidget->isStabLoaded()){
        if(ramViewer != NULL){
            mainLayout->removeWidget(ramViewer);
            delete ramViewer;
        }
        ramViewer = new HexView(0, 0x1c, mac, mainWidget);
        mainLayout->addWidget(ramViewer);
    }
    ramViewer->Refresh();
}

void structures_window::onSelectedObject(Word start, Word end){
    if(ramViewer != NULL){
        mainLayout->removeWidget(ramViewer);
        delete ramViewer;
    }
    ramViewer = new HexView(start, end, mac, mainWidget);
    mainLayout->addWidget(ramViewer);
}