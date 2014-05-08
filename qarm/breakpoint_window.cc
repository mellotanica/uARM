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

#include <QCloseEvent>

#include "qarm/QLine.h"
#include "services/debug_session.h"

breakpoint_window::breakpoint_window(QWidget * parent, Qt::WindowFlags flags):
    QMainWindow(parent, flags)
{
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout();

    addWidget = new AddBreakpointDialog(this);

    QWidget *buttonsW = new QWidget(mainWidget);
    QHBoxLayout *buttonsL = new QHBoxLayout(buttonsW);

    QWidget *buttonsLeftW = new QWidget(buttonsW);
    QGridLayout *buttonsLeftL = new QGridLayout(buttonsLeftW);

    QWidget *buttonsRightW = new QWidget(buttonsW);
    QGridLayout *buttonsRightL = new QGridLayout(buttonsRightW);

    removeBreakpointAction = new QAction("Remove Breakpoint", this);
    connect(removeBreakpointAction, SIGNAL(triggered()), this, SLOT(onRemoveBreakpoint()));

    add = new QToolButton(buttonsLeftW);
    connect(add, SIGNAL(clicked()), this, SLOT(onAddBreakpoint()));

    remove = new QToolButton(buttonsRightW);
    connect(remove, SIGNAL(clicked()), this, SLOT(onRemoveBreakpoint()));

    add->setText("Add");

    remove->setText("Remove");

    buttonsLeftL->addWidget(add, 0, 0);
    buttonsLeftL->addWidget(new QFLine(false), 1, 0);
    buttonsLeftW->setLayout(buttonsLeftL);

    buttonsRightL->addWidget(new QFLine(false), 0, 0);
    buttonsRightL->addWidget(remove, 1, 0);
    buttonsRightW->setLayout(buttonsRightL);

    buttonsL->addWidget(buttonsLeftW);
    buttonsL->addWidget(buttonsRightW);

    buttonsW->setLayout(buttonsL);

    //EDIT: setup buttons

    breakpointView = new QTreeView;
    breakpointView->setRootIsDecorated(false);
    breakpointView->setContextMenuPolicy(Qt::ActionsContextMenu);
    breakpointView->setAlternatingRowColors(false);
    breakpointView->addAction(removeBreakpointAction);

    mainLayout->addWidget(addWidget);
    mainLayout->addWidget(buttonsW);
    mainLayout->addWidget(breakpointView);

    mainWidget->setLayout(mainLayout);

    add->setEnabled(false);
    remove->setEnabled(false);
    removeBreakpointAction->setEnabled(false);

    hide();

    connect(DebuggerHolder::getInstance()->getDebugSession(), SIGNAL(stabUpdated()), this, SLOT(symtabReady()));
    connect(DebuggerHolder::getInstance()->getDebugSession(), SIGNAL(stabUnavavilable()), this, SLOT(symtabMissing()));

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
    clearBPs();
}

void breakpoint_window::clearBPs(){
    if(breakpointView->selectionModel() != NULL)
        breakpointView->selectionModel()->clear();
}

void breakpoint_window::onRemoveBreakpoint(){
    QModelIndexList idx = breakpointView->selectionModel()->selectedRows();
    if (!idx.isEmpty())
        stoppointList->Remove(idx.first().row());
}

void breakpoint_window::onAddBreakpoint(){
    AddressRange r(addWidget->getASID(), addWidget->getStartAddress(), addWidget->getStartAddress());
    stoppointList->Add(r, AM_EXEC);
}

void breakpoint_window::symtabReady(){
    add->setEnabled(true);
    remove->setEnabled(true);
    removeBreakpointAction->setEnabled(true);
    stoppointList = new StoppointListModel(DebuggerHolder::getInstance()->getDebugSession()->getBreakpoints(),
                                           "Breakpoints", 'B', this);
    breakpointView->setModel(stoppointList);
}

void breakpoint_window::symtabMissing(){
    add->setEnabled(false);
    remove->setEnabled(false);
    removeBreakpointAction->setEnabled(false);
    if(breakpointView->selectionModel() != NULL)
        breakpointView->selectionModel()->clear();
    delete stoppointList;
}

void breakpoint_window::updateContent(){
    clearBPs();
    addWidget->updateContent();
}
