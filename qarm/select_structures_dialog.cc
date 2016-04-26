/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uMPS - A general purpose computer system simulator
 *
 * Copyright (C) 2010 Tomislav Jonjic
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

#include "qarm/select_structures_dialog.h"

#include <QGridLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QFontMetrics>
#include <QItemSelectionModel>

#include "services/symbol_table.h"
#include "services/debug_session.h"
#include "armProc/machine_config.h"
#include "qarm/symbol_table_model.h"

SelectStructuresDialog::SelectStructuresDialog(QWidget* parent)
    : QWidget(parent)
{
    QGridLayout* layout = new QGridLayout(this);

    symbolTableView = new QTreeView(this);
    symbolTableView->setSortingEnabled(true);
    symbolTableView->sortByColumn(0, Qt::AscendingOrder);
    symbolTableView->setAlternatingRowColors(true);
    symbolTableView->resizeColumnToContents(0);
    symbolTableView->hideColumn(SymbolTableModel::COLUMN_END_ADDRESS);

    updateContent();

    layout->addWidget(symbolTableView, 1, 0, 1, 5);

    setWindowTitle("Show Object Contents");
    resize(kInitialWidth, kInitialHeight);
}

void SelectStructuresDialog::updateContent(){
    if(DebuggerHolder::getInstance()->getDebugSession()->getSymbolTable() != NULL){
        stab = DebuggerHolder::getInstance()->getDebugSession()->getSymbolTable();
        QAbstractTableModel* stabModel = new SymbolTableModel(this);
        proxyModel = new SortFilterSymbolTableModel(Symbol::TYPE_OBJECT, this);
        proxyModel->setSourceModel(stabModel);

        symbolTableView->setModel(proxyModel);

        disconnect(this, SLOT(onSelectionChanged(QItemSelection)));
        connect(symbolTableView->selectionModel(),
                SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                this,
                SLOT(onSelectionChanged(const QItemSelection&)));

        onSelectionChanged(symbolTableView->selectionModel()->selection());
    } else {
        symbolTableView->setModel(new EmptySymbolTableModel);
        disconnect(this, SLOT(onSelectionChanged(QItemSelection)));
        stab = NULL;
        start = end = 0;
        label = QString();
    }
}

void SelectStructuresDialog::onSelectionChanged(const QItemSelection& selected)
{
    QModelIndexList indexes = selected.indexes();
    if (!indexes.isEmpty()) {
        int row = proxyModel->mapToSource(indexes[0]).row();
        const Symbol* symbol = stab->Get(row);
        start = symbol->getStart();
        end = symbol->getEnd();
        label = QString(symbol->getName());
        emit selectedObject(start, end);
    }
}

void SelectStructuresDialog::triggerOpenRam(){
    if(!label.isEmpty()){
        Word offset = end - start;
        emit openRam(start, offset, label, true);
    }
}

void SelectStructuresDialog::triggerOpenRam(Word addr){
    emit openRam(addr, 0, "", true);
}
