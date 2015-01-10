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

#include "tlb_window.h"

#include <QCloseEvent>
#include <QHeaderView>

#include "qarm/register_item_delegate.h"

tlb_window::tlb_window(machine *mac, QWidget * parent, Qt::WindowFlags flags):
    QMainWindow(parent, flags)
{
    tlbView = new QTableView;
    TLBModel *innermodel = new TLBModel(mac, this);
    connect(this, SIGNAL(onMachineReset()), innermodel, SLOT(onMachineReset()));
    connect(innermodel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), tlbView, SLOT(dataChanged(QModelIndex,QModelIndex)));
    tlbView->setModel(innermodel);
    tlbView->setSelectionMode(QAbstractItemView::SingleSelection);
    tlbView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tlbView->horizontalHeader()->setStretchLastSection(true);
    tlbView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tlbView->horizontalHeader()->setHighlightSections(false);
    tlbView->verticalHeader()->setHighlightSections(false);
    tlbView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    tlbView->setAlternatingRowColors(true);
    tlbView->setItemDelegate(new RIDelegateHex(this));
    tlbView->resizeRowsToContents();

    setCentralWidget(tlbView);
    setWindowTitle("TLB");
}

tlb_window::~tlb_window(){
}

void tlb_window::closeEvent(QCloseEvent *event){
    emit hiding();
    hide();
    event->ignore();
}
