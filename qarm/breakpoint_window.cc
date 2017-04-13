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

breakpoint_window::breakpoint_window(machine *mac, QWidget * parent, Qt::WindowFlags flags):
    QMainWindow(parent, flags)
{
    QWidget *mainWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);

    // FIXME: slow
    addWidget = new AddBreakpointDialog(mainWidget);

    QWidget *buttonsW = new QWidget(mainWidget);
    QHBoxLayout *buttonsL = new QHBoxLayout();

    QWidget *buttonsLeftW = new QWidget(buttonsW);
    QGridLayout *buttonsLeftL = new QGridLayout();

    QWidget *buttonsCenterW = new QWidget(buttonsW);
    QGridLayout *buttonsCenterL = new QGridLayout();

    QWidget *buttonsRightW = new QWidget(buttonsW);
    QGridLayout *buttonsRightL = new QGridLayout();

    removeBreakpointAction = new QAction("Remove Breakpoint", this);
    connect(removeBreakpointAction, SIGNAL(triggered()), this, SLOT(onRemoveBreakpoint()));

    add = new QToolButton(buttonsLeftW);
    connect(add, SIGNAL(clicked()), this, SLOT(onAddBreakpoint()));

    remove = new QToolButton(buttonsRightW);
    connect(remove, SIGNAL(clicked()), this, SLOT(onRemoveBreakpoint()));

    add->setText("Add");
    add->setAccessibleName("Add Breakpoint");

    remove->setText("Remove");
    remove->setAccessibleName("Remove Breakpoint");

    breakpointsActive = new QCheckBox("Stop on Breakpoint", buttonsW);
    breakpointsActive->setAccessibleName(breakpointsActive->text());
    connect(breakpointsActive, SIGNAL(toggled(bool)), mac, SLOT(toggleBP(bool)));
    breakpointsActive->setChecked(true);
    buttonsCenterL->addWidget(breakpointsActive);

    breakOnTLB = new QCheckBox("Stop on TLB change", buttonsW);
    breakOnTLB->setAccessibleName(breakOnTLB->text());
    connect(breakOnTLB, SIGNAL(toggled(bool)), mac, SLOT(toggleTLBPause(bool)));
    breakOnTLB->setChecked(MC_Holder::getInstance()->getConfig()->getStopOnTLBChange());
    buttonsCenterL->addWidget(breakOnTLB);
    buttonsCenterW->setLayout(buttonsCenterL);

    buttonsLeftL->addWidget(add, 0, 0);
    buttonsLeftL->addWidget(new QFLine(false), 1, 0);
    buttonsLeftW->setLayout(buttonsLeftL);

    buttonsRightL->addWidget(new QFLine(false), 0, 0);
    buttonsRightL->addWidget(remove, 1, 0);
    buttonsRightW->setLayout(buttonsRightL);

    buttonsL->addWidget(buttonsLeftW);
    buttonsL->addWidget(buttonsCenterW);
    buttonsL->addWidget(buttonsRightW);

    buttonsW->setLayout(buttonsL);

    breakpointView = new QTreeView(mainWidget);
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
    setWindowTitle("Breakpoints");
}

breakpoint_window::~breakpoint_window(){

}

void breakpoint_window::closeEvent(QCloseEvent *event){
    emit hiding();
    hide();
    event->ignore();
}

void breakpoint_window::reset(){
    if(DebuggerHolder::getInstance()->getDebugSession()->getSymbolTable() != activeStab)
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
    activeStab = DebuggerHolder::getInstance()->getDebugSession()->getSymbolTable();
}

void breakpoint_window::symtabMissing(){
    add->setEnabled(false);
    remove->setEnabled(false);
    removeBreakpointAction->setEnabled(false);
    if(breakpointView->selectionModel() != NULL)
        breakpointView->selectionModel()->clear();
    delete stoppointList;
    activeStab = NULL;
}

void breakpoint_window::updateContent(){
    clearBPs();
    addWidget->updateContent();
}
