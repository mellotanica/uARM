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

#include "qarm/ui_utils.h"

#include <QAbstractItemModel>
#include <QTreeView>

#include "facilities/arch.h"
#include "services/symbol_table.h"

static const char* const kBiosSpaceName = "ExecROM";
//static const char* const kBootSpaceName = "BootROM";

const char* GetSymbolicAddress(const SymbolTable* symbolTable,
                               Word               asid,
                               Word               address,
                               bool               onlyFunctions,
                               SWord*             offset)
{
    if (address >= RAM_BASE) {
        return symbolTable->Probe(asid, address, !onlyFunctions, offset);
    } else {
        *offset = address - KSEGOS_BASE;
        return kBiosSpaceName;
    }
}

QString FormatAddress(Word addr)
{
    return (QString("0x%1.%2")
            .arg((quint32) (addr >> 16), 4, 16, QChar('0'))
            .arg((quint32) (addr & 0x0000ffffU), 4, 16, QChar('0')));
}

void SetFirstColumnSpanned(QTreeView* treeView, bool setting)
{
    QAbstractItemModel* model = treeView->model();
    QModelIndex rootIndex = treeView->rootIndex();
    int rows = model->rowCount(rootIndex);
    for (int i = 0; i < rows; i++)
        treeView->setFirstColumnSpanned(i, rootIndex, setting);
}
