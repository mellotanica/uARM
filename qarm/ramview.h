/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uARM
 *
 * Copyright (C) 2013 Marco Melletti
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

#ifndef QARM_RAMVIEW_H
#define QARM_RAMVIEW_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QValidator>
#include "armProc/machine.h"
#include "qarm/hex_view.h"
#include "qarm/QLine.h"

class ramView : public QWidget
{
    Q_OBJECT
public:
    explicit ramView(machine *mac, QWidget *parent = 0);
    ramView(machine *mac, Word start, Word end, QString label, QWidget *parent = 0);

signals:
    

public slots:
    void visualize();
    void update();

private:
    machine *mac;
    Word startAddr = 0, endAddr = 0;

    void newRamLabel(QWidget *parent = 0);
    void initRamView(machine *mac);

    HexView *ramViewer = NULL;
    QVBoxLayout *mainLayout;
    QLineEdit *startEd, *endEd, *ramLabel = NULL;
    QPushButton *visualizeB;
    QString labelText;
};

#endif // QARM_RAMVIEW_H
