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

#ifndef QARM_SELECT_STRUCTURES_DIALOG_H
#define QARM_SELECT_STRUCTURES_DIALOG_H

#include <QWidget>
#include <QItemSelection>
#include <QTreeView>

#include "armProc/types.h"

class SymbolTable;
class SortFilterSymbolTableModel;

class SelectStructuresDialog : public QWidget {
    Q_OBJECT

public:
    SelectStructuresDialog(QWidget* parent = 0);

    bool isStabLoaded() {return (stab != NULL);}
signals:
    void selectedObject(Word start, Word end);
    void openRam(Word start, Word end, QString label);

public slots:
    void updateContent();
    void triggerOpenRam();

private:
    static const int kInitialWidth = 380;
    static const int kInitialHeight = 340;

    QTreeView *symbolTableView;

    Word start, end;
    QString label;

    const SymbolTable* stab;
    SortFilterSymbolTableModel* proxyModel;

private Q_SLOTS:
    void onSelectionChanged(const QItemSelection& selected);
};

#endif // QARM_SELECT_STRUCTURES_DIALOG_H
