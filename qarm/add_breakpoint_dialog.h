/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uMPS - A general purpose computer system simulator
 *
 * Copyright (C) 2010 Tomislav Jonjic
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

#ifndef QMPS_ADD_BREAKPOINT_DIALOG_H
#define QMPS_ADD_BREAKPOINT_DIALOG_H

#include <QWidget>
#include <QItemSelection>
#include <QTreeView>

#include "armProc/types.h"

class AddressLineEdit;
class AsidLineEdit;
class SymbolTable;
class SortFilterSymbolTableModel;

class AddBreakpointDialog : public QWidget {
    Q_OBJECT

public:
    AddBreakpointDialog(QWidget* parent = 0);

    Word getStartAddress() const;
    Word getASID() const;

public slots:
    void updateContent();

private:
    static const int kInitialWidth = 380;
    static const int kInitialHeight = 340;

    AsidLineEdit* asidEditor;
    AddressLineEdit* addressEditor;
    QTreeView *symbolTableView;

    const SymbolTable* const stab;
    SortFilterSymbolTableModel* proxyModel;

private Q_SLOTS:
    void onSelectionChanged(const QItemSelection& selected);
};

#endif // QMPS_ADD_BREAKPOINT_DIALOG_H
