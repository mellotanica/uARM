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

#ifndef QARM_MACHINE_CONFIG_DIALOG_H
#define QARM_MACHINE_CONFIG_DIALOG_H

#include <QDialog>

#include "armProc/machine_config.h"

class QListWidget;
class QStackedLayout;
class QSpinBox;
class QComboBox;
class QLineEdit;
class QCheckBox;
class AsidLineEdit;

class MachineConfigDialog : public QDialog {
    Q_OBJECT

public:
    MachineConfigDialog(MachineConfig* config, QWidget* parent = 0);

private:
    QWidget* createGeneralTab();
    QWidget* createDeviceTab();
    void registerDeviceClass(const QString& label,
                             const QString& icon,
                             unsigned int   devClassIndex,
                             const QString& deviceClassName,
                             const QString& deviceName,
                             bool           selected = false);

    MachineConfig* const config;

    QSpinBox* cpuSpinner;
    QSpinBox* clockRateSpinner;
    QSpinBox* refreshRateSpinner;
    QCheckBox *refreshEnabledBox;
    QComboBox* tlbSizeList;
    QSpinBox* ramSizeSpinner;
    QCheckBox* stopOnInterruptBox;
    QCheckBox* coreBootCheckBox;
    AsidLineEdit* stabAsidEdit;

    struct {
        const char* description;
        QLineEdit* lineEdit;
    } romFileInfo[N_ROM_TYPES];

    QListWidget* devClassView;
    QStackedLayout* devFileChooserStack;

private Q_SLOTS:
    void getROMFileName(int index);

    void onDeviceClassChanged();

    void saveConfigChanges();
};

#endif // QARM_MACHINE_CONFIG_DIALOG_H
