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

#include "breakpoint_window.h"
#include "qarm/QLine.h"
#include <QCloseEvent>

breakpoint_window::breakpoint_window(QWidget * parent, Qt::WindowFlags flags):
    QMainWindow(parent, flags)
{
    mainWidget = new QWidget(this);
    mainLayout = new QVBoxLayout();

    addWidget = new AddBreakpointDialog(this);

    scritta = new QLabel("pezzo sotto");

    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Discard);

    mainLayout->addWidget(addWidget);
    mainLayout->addWidget(new QFLine(false));
    mainLayout->addWidget(buttons);
    mainLayout->addWidget(new QFLine(false));
    mainLayout->addWidget(scritta);

    mainWidget->setLayout(mainLayout);

    hide();

    setCentralWidget(mainWidget);
}

breakpoint_window::~breakpoint_window(){

}

void breakpoint_window::closeEvent(QCloseEvent *event){
    emit hiding();
    hide();
    event->ignore();
}

void breakpoint_window::reset(){
    scritta->setText("resettato!");
}

bool breakpoint_window::isBPactive(Word addr, Word asid){

}

void breakpoint_window::addBP(Word addr, Word asid){

}

void breakpoint_window::clearBPs(){

}

void breakpoint_window::updateContent(){
    clearBPs();
    addWidget->updateContent();
}
