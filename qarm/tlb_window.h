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

#ifndef TLB_WINDOW_H
#define TLB_WINDOW_H

#include <QMainWindow>
#include <QDialogButtonBox>
#include <QTableView>
#include <QHBoxLayout>
#include <QLabel>
#include "armProc/machine.h"
#include "qarm/tlb_model.h"

class tlb_window : public QMainWindow
{
    Q_OBJECT
public:
    tlb_window(machine *mac, QWidget * parent = 0, Qt::WindowFlags flags = 0);
    ~tlb_window();

protected:
    virtual void closeEvent(QCloseEvent* event);

private slots:
    void updateDetails(QString details);

signals:
    void hiding();
    void onMachineReset();

private:
    QTableView *tlbView;
    QHBoxLayout *hLayout;
    QWidget *mainWidget;
    QLabel *detailsLabel;
};

#endif // TLB_WINDOW_H
