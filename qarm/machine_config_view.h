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

#ifndef QMPS_MACHINE_CONFIG_VIEW_H
#define QMPS_MACHINE_CONFIG_VIEW_H

#include <QWidget>

class QLabel;

class MachineConfigView : public QWidget {
    Q_OBJECT

public:
    MachineConfigView(QWidget* parent = 0);

public Q_SLOTS:
    void Update();

private:
    static QString checkedFileName(const QString& fileName);

    QLabel* numCpusLabel;
    QLabel* clockRateLabel;
    QLabel* tlbSizeLabel;
    QLabel* ramSizeLabel;

    QLabel* bootstrapROMLabel;
    QLabel* executionROMLabel;

    QLabel* loadCoreLabel;
    QLabel* coreFileLabel;

    QLabel* stabLabel;
    QLabel* stabAsidLabel;
};

#endif // QMPS_MACHINE_CONFIG_VIEW_H
