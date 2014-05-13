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

#ifndef BREAKPOINT_WINDOW_H
#define BREAKPOINT_WINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QToolButton>
#include <QCheckBox>
#include "armProc/const.h"
#include "armProc/machine.h"
#include "qarm/add_breakpoint_dialog.h"
#include "qarm/stoppoint_list_model.h"

#include <QAction>

class breakpoint_window : public QMainWindow
{
    Q_OBJECT
public:
    breakpoint_window(machine *mac, QWidget * parent = 0, Qt::WindowFlags flags = 0);
    ~breakpoint_window();

public slots:
    void reset();

protected:
    virtual void closeEvent(QCloseEvent* event);

signals:
    void hiding();

private slots:
    void updateContent();
    void symtabReady();
    void symtabMissing();
    void onRemoveBreakpoint();
    void onAddBreakpoint();

private:
    QAction *removeBreakpointAction;

    AddBreakpointDialog *addWidget;
    QToolButton *add, *remove;
    StoppointListModel *stoppointList;
    QTreeView *breakpointView;
    QCheckBox *breakpointsActive;

    SymbolTable *activeStab;

    void addBP(Word addr, Word asid);
    void addBP(Word addr) { addBP(addr, 0); }

    void clearBPs();
};

#endif // BREAKPOINT_WINDOW_H
