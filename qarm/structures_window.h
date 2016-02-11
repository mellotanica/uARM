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

#ifndef STRUCTURES_WINDOW_H
#define STRUCTURES_WINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include "armProc/const.h"
#include "armProc/types.h"
#include "armProc/machine.h"
#include "qarm/select_structures_dialog.h"
#include "qarm/hex_view.h"

class structures_window : public QMainWindow
{
    Q_OBJECT
public:
    structures_window(machine *mac, QWidget * parent = 0, Qt::WindowFlags flags = 0);
    ~structures_window();

protected:
    virtual void closeEvent(QCloseEvent* event);

signals:
    void hiding();

private slots:
    void updateContent();
    void onSelectedObject(Word start, Word end);

private:
    machine *mac;

    QWidget *mainWidget;
    QVBoxLayout *mainLayout;

    SelectStructuresDialog *selectWidget;
    HexView *ramViewer = NULL;

    SymbolTable *activeStab;
};

#endif // STRUCTURES_WINDOW_H
