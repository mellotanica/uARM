/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uMPS - A general purpose computer system simulator
 *
 * Copyright (C) 2010, 2011 Tomislav Jonjic
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

#ifndef QARM_MACHINE_CONFIG_DIALOG_PRIV_H
#define QARM_MACHINE_CONFIG_DIALOG_PRIV_H

#include <QWidget>

#include "armProc/arch.h"

class QLineEdit;
class QCheckBox;
class MacIdEdit;

class DeviceFileChooser : public QWidget {
    Q_OBJECT

public:
    DeviceFileChooser(const QString& deviceClassName,
                      const QString& deviceName,
                      unsigned int   line,
                      QWidget*       parent = 0);

    QString getDeviceFile(unsigned int devNo);
    bool IsDeviceEnabled(unsigned int devNo);

public Q_SLOTS:
    void Save();

private:
    unsigned int il;
    QString deviceName;
    QLineEdit* fileNameEdit[N_DEV_PER_IL];
    QCheckBox* enabledCB[N_DEV_PER_IL];

private Q_SLOTS:
    void browseDeviceFile(int devNo);
};

class NetworkConfigWidget : public QWidget {
    Q_OBJECT

public:
    NetworkConfigWidget(QWidget* parent = 0);

public Q_SLOTS:
    void Save();

private:
    QCheckBox* enabledCB[N_DEV_PER_IL];
    QLineEdit* fileEdit[N_DEV_PER_IL];
    QCheckBox* fixedMacId[N_DEV_PER_IL];
    MacIdEdit* macIdEdit[N_DEV_PER_IL];

private Q_SLOTS:
    void browseDeviceFile(int devNo);
};

#endif // QARM_MACHINE_CONFIG_DIALOG_PRIV_H
