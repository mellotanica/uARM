/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uMPS - A general purpose computer system simulator
 *
 * Copyright (C) 2010, 2011 Tomislav Jonjic
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

#ifndef QARM_MACHINE_CONFIG_DIALOG_CC
#define QARM_MACHINE_CONFIG_DIALOG_CC

#include "qarm/machine_config_dialog.h"

#include <cassert>

#include <QSignalMapper>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QStackedLayout>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>

#include "facilities/arch.h"
#include "qarm/address_line_edit.h"
#include "qarm/mac_id_edit.h"
#include "qarm/machine_config_dialog_priv.h"
#include "qarm/procdisplay.h"

MachineConfigDialog::MachineConfigDialog(MachineConfig* config, QWidget* parent)
    : QDialog(parent),
      config(config)
{
    setWindowTitle("Machine Configuration");
    setWindowIcon(QIcon(LIB_PATH "icons/config.png"));

    QTabWidget* tabWidget = new QTabWidget(this);
    tabWidget->addTab(createGeneralTab(tabWidget), "&General");
    tabWidget->addTab(createDeviceTab(tabWidget), "&Devices");

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                       QDialogButtonBox::Cancel, this);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(this, SIGNAL(accepted()), this, SLOT(saveConfigChanges()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QLayout* layout = new QVBoxLayout();
    layout->addWidget(tabWidget);
    layout->addWidget(buttonBox);

    setLayout(layout);
}

QWidget* MachineConfigDialog::createGeneralTab(QWidget *parent)
{
    QWidget* tabWidget = new QWidget(parent);
    tabWidget->setAccessibleName("General Settings");

    QGridLayout* layout = new QGridLayout(tabWidget);
    layout->setContentsMargins(11, 13, 11, 11);

    layout->addWidget(new QLabel("<b>Hardware</b>", tabWidget), 0, 0, 1, 3);

    /* STATIC: only one core
    layout->addWidget(new QLabel("Processors:"), 1, 1);
    cpuSpinner = new QSpinBox();
    cpuSpinner->setMinimum(MachineConfig::MIN_CPUS);
    cpuSpinner->setMaximum(MachineConfig::MAX_CPUS);
    cpuSpinner->setValue(config->getNumProcessors());
    layout->addWidget(cpuSpinner, 1, 3);*/

    layout->addWidget(new QLabel("Default Clock Rate (MHz):", tabWidget), 1, 1);
    clockRateSpinner = new QSpinBox(tabWidget);
    clockRateSpinner->setAccessibleName("Default Clock Rate (MHz):");
    clockRateSpinner->setAccessibleDescription("This will change the relative time taken by devices to operate");
    clockRateSpinner->setToolTip(clockRateSpinner->accessibleDescription());
    clockRateSpinner->setMinimum(MachineConfig::MIN_CLOCK_RATE);
    clockRateSpinner->setMaximum(MachineConfig::MAX_CLOCK_RATE);
    clockRateSpinner->setValue(config->getClockRate());
    layout->addWidget(clockRateSpinner, 1, 2);

    layout->addWidget(new QLabel("GUI Refresh Rate:", tabWidget), 2, 1);
    refreshRateSpinner = new QSpinBox(tabWidget);
    refreshRateSpinner->setAccessibleName("GUI Refresh Rate");
    refreshRateSpinner->setAccessibleDescription("Number of CPU cycles between each refresh");
    refreshRateSpinner->setToolTip(refreshRateSpinner->accessibleDescription());
    refreshRateSpinner->setMinimum(MachineConfig::MIN_REFRESH_RATE);
    refreshRateSpinner->setMaximum(MachineConfig::MAX_REFRESH_RATE);
    refreshRateSpinner->setValue(config->getRefreshRate());
    layout->addWidget(refreshRateSpinner, 2, 2);

    refreshEnabledBox = new QCheckBox(tabWidget);
    refreshEnabledBox->setAccessibleName("Enable constant refresh");
    refreshEnabledBox->setAccessibleDescription("If selected, each GUI component gets refreshed each Refresh Rate cycles, else refresh will happen on execution stop");
    refreshEnabledBox->setToolTip(refreshEnabledBox->accessibleDescription());
    refreshEnabledBox->setText(refreshEnabledBox->accessibleName());
    refreshEnabledBox->setChecked(!config->getRefreshOnPause());
    if(!refreshEnabledBox->isChecked())
        refreshRateSpinner->setEnabled(false);
    layout->addWidget(refreshEnabledBox, 2, 3);

    connect(refreshEnabledBox, SIGNAL(clicked(bool)), refreshRateSpinner, SLOT(setEnabled(bool)));

    layout->addWidget(new QLabel("RAM Size (Frames):", tabWidget), 3, 1);
    ramSizeSpinner = new QSpinBox(tabWidget);
    ramSizeSpinner->setAccessibleName("Ram Size (Frames)");
    ramSizeSpinner->setAccessibleDescription("Total size of accessible RAM expressed in frames");
    ramSizeSpinner->setToolTip(ramSizeSpinner->accessibleDescription());
    ramSizeSpinner->setMinimum(MachineConfig::MIN_RAM);
    ramSizeSpinner->setMaximum(MachineConfig::MAX_RAM);
    ramSizeSpinner->setValue(config->getRamSize());
    layout->addWidget(ramSizeSpinner, 3, 2);

    layout->addWidget(new QLabel("TLB Size (Entries):", tabWidget), 4, 1);
    tlbSizeList = new QComboBox(tabWidget);
    tlbSizeList->setAccessibleName("TLB Size (Entries)");
    tlbSizeList->setAccessibleDescription("Size of the TLB cache (power of 2 between 4 and 64)");
    tlbSizeList->setToolTip(tlbSizeList->accessibleDescription());
    int currentIndex = 0;
    for (unsigned int val = MachineConfig::MIN_TLB_SIZE; val <= MachineConfig::MAX_TLB_SIZE; val <<= 1) {
        tlbSizeList->addItem(QString::number(val));
        if (config->getTLBSize() == val)
            tlbSizeList->setCurrentIndex(currentIndex);
        currentIndex++;
    }
    layout->addWidget(tlbSizeList, 4, 2);

    stopOnInterruptBox = new QCheckBox("Pause execution on Exception", tabWidget);
    stopOnInterruptBox->setAccessibleName("Pause execution on Exception");
    stopOnInterruptBox->setChecked(config->getStopOnException());
    layout->addWidget(stopOnInterruptBox, 5, 1, 1, 3);

    QSignalMapper* fileChooserMapper = new QSignalMapper(this);
    connect(fileChooserMapper, SIGNAL(mapped(int)), this, SLOT(getROMFileName(int)));
    QPushButton* fileChooserButton;

    layout->addWidget(new QLabel("<b>BIOS</b>", tabWidget), 6, 0, 1, 3);

    layout->addWidget(new QLabel("Execution ROM:", tabWidget), 7, 1);
    romFileInfo[ROM_TYPE_BIOS].description = "Execution ROM";
    romFileInfo[ROM_TYPE_BIOS].lineEdit = new QLineEdit(tabWidget);
    romFileInfo[ROM_TYPE_BIOS].lineEdit->setAccessibleName("Execution ROM");
    romFileInfo[ROM_TYPE_BIOS].lineEdit->setAccessibleDescription("Path to Execution ROM file (*.rom.uarm)");
    romFileInfo[ROM_TYPE_BIOS].lineEdit->setToolTip(romFileInfo[ROM_TYPE_BIOS].lineEdit->accessibleDescription());
    layout->addWidget(romFileInfo[ROM_TYPE_BIOS].lineEdit, 7, 2, 1, 2);
    romFileInfo[ROM_TYPE_BIOS].lineEdit->setText(config->getROM(ROM_TYPE_BIOS).c_str());
    fileChooserButton = new QPushButton("Browse...", tabWidget);
    fileChooserButton->setAccessibleName("Browse for Execution ROM");
    connect(fileChooserButton, SIGNAL(clicked()), fileChooserMapper, SLOT(map()));
    fileChooserMapper->setMapping(fileChooserButton, ROM_TYPE_BIOS);
    layout->addWidget(fileChooserButton, 7, 4);

    layout->addWidget(new QLabel("<b>Boot</b>", tabWidget), 8, 0, 1, 3);

    /* STATIC: when dynamic loading will be possible remove this!
    coreBootCheckBox = new QCheckBox("Load core file");
    coreBootCheckBox->setChecked(config->isLoadCoreEnabled());

    coreBootCheckBox->setEnabled(false);
    layout->addWidget(coreBootCheckBox, 11, 1, 1, 3);*/

    layout->addWidget(new QLabel("Core file:", tabWidget), 9, 1);
    romFileInfo[ROM_TYPE_CORE].description = "Core";
    romFileInfo[ROM_TYPE_CORE].lineEdit = new QLineEdit(tabWidget);
    romFileInfo[ROM_TYPE_CORE].lineEdit->setAccessibleName("Core file");
    romFileInfo[ROM_TYPE_CORE].lineEdit->setAccessibleDescription("Path to Core file (*.core.uarm)");
    romFileInfo[ROM_TYPE_CORE].lineEdit->setToolTip(romFileInfo[ROM_TYPE_CORE].lineEdit->accessibleDescription());
    layout->addWidget(romFileInfo[ROM_TYPE_CORE].lineEdit, 9, 2, 1, 2);
    romFileInfo[ROM_TYPE_CORE].lineEdit->setText(config->getROM(ROM_TYPE_CORE).c_str());
    fileChooserButton = new QPushButton("Browse...", tabWidget);
    fileChooserButton->setAccessibleName("Browse for Core file");
    connect(fileChooserButton, SIGNAL(clicked()), fileChooserMapper, SLOT(map()));
    fileChooserMapper->setMapping(fileChooserButton, ROM_TYPE_CORE);
    layout->addWidget(fileChooserButton, 9, 4);

    layout->addWidget(new QLabel("<b>Debugging Support</b>", tabWidget), 10, 0, 1, 3);

    layout->addWidget(new QLabel("Symbol Table:", tabWidget), 11, 1);

    romFileInfo[ROM_TYPE_STAB].description = "Symbol Table";
    romFileInfo[ROM_TYPE_STAB].lineEdit = new QLineEdit(tabWidget);
    romFileInfo[ROM_TYPE_STAB].lineEdit->setAccessibleName("Symbol Table");
    romFileInfo[ROM_TYPE_STAB].lineEdit->setAccessibleDescription("Path to Symbol Table file (*.stab.uarm)");
    romFileInfo[ROM_TYPE_STAB].lineEdit->setToolTip(romFileInfo[ROM_TYPE_STAB].lineEdit->accessibleDescription());
    layout->addWidget(romFileInfo[ROM_TYPE_STAB].lineEdit, 11, 2, 1, 2);
    romFileInfo[ROM_TYPE_STAB].lineEdit->setText(config->getROM(ROM_TYPE_STAB).c_str());
    fileChooserButton = new QPushButton("Browse...", tabWidget);
    fileChooserButton->setAccessibleName("Browse for Symbol Table file");
    connect(fileChooserButton, SIGNAL(clicked()), fileChooserMapper, SLOT(map()));
    fileChooserMapper->setMapping(fileChooserButton, ROM_TYPE_STAB);
    layout->addWidget(fileChooserButton, 11, 4);

    layout->addWidget(new QLabel("Symbol Table ASID:", tabWidget), 12, 1);
    stabAsidEdit = new AsidLineEdit(tabWidget);
    stabAsidEdit->setAccessibleName("Symbol Table ASID");
    stabAsidEdit->setAccessibleDescription("Default Symbol Table ASID");
    stabAsidEdit->setToolTip(stabAsidEdit->accessibleDescription());
    stabAsidEdit->setMaximumWidth(100);
    stabAsidEdit->setAsid(config->getSymbolTableASID());
    layout->addWidget(stabAsidEdit, 12, 2);

    /*layout->setColumnMinimumWidth(0, 10);
    layout->setColumnMinimumWidth(2, 10);
    layout->setColumnMinimumWidth(3, 100);
    layout->setColumnMinimumWidth(5, 75);

    layout->setRowMinimumHeight(5, 11);
    layout->setRowMinimumHeight(9, 11);
    layout->setRowMinimumHeight(13, 11);

    layout->setRowStretch(17, 1);
    layout->setColumnStretch(4, 1);*/

    romFileInfo[ROM_TYPE_STAB].lineEdit->setEnabled(true);
    fileChooserButton->setEnabled(true);
    stabAsidEdit->setEnabled(true);

    return tabWidget;
}

QWidget* MachineConfigDialog::createDeviceTab(QWidget *parent)
{
    static const int TAB_MARGIN_TOP = 3;
    static const int TAB_MARGIN_BOTTOM = 3;
    static const int TAB_MARGIN_LEFT = 3;
    static const int TAB_MARGIN_RIGHT = 3;

    QWidget* tab = new QWidget(parent);
    tab->setAccessibleName("Devices Settings");
    QVBoxLayout* tabLayout = new QVBoxLayout(tab);

    tabLayout->setContentsMargins(TAB_MARGIN_TOP,
                                  TAB_MARGIN_BOTTOM,
                                  TAB_MARGIN_LEFT,
                                  TAB_MARGIN_RIGHT);

    devClassView = new QComboBox(tab);
    devClassView->setAccessibleName("Device Type");

    tabLayout->addWidget(devClassView);

    devFileChooserStack = new QStackedLayout();
    tabLayout->addLayout(devFileChooserStack);

    connect(devClassView, SIGNAL(currentIndexChanged(int)), devFileChooserStack, SLOT(setCurrentIndex(int)));

    registerDeviceClass("Disks\n Interrupt Line 3",
                        LIB_PATH "icons/disk-32.png",
                        EXT_IL_INDEX(IL_DISK),
                        "Disks", "Disk",
                        tab, true);

    registerDeviceClass("Tapes\n Interrupt Line 4",
                        LIB_PATH "icons/tape-32.png",
                        EXT_IL_INDEX(IL_TAPE),
                        "Tapes", "Tape", tab);

    registerDeviceClass("Network\n Interrupt Line 5",
                        LIB_PATH "icons/network-32.png",
                        EXT_IL_INDEX(IL_ETHERNET),
                        "Network Interfaces", "Net", tab);

    registerDeviceClass("Printers\n Interrupt Line 6",
                        LIB_PATH "icons/printer-32.png",
                        EXT_IL_INDEX(IL_PRINTER),
                        "Printers", "Printer", tab);

    registerDeviceClass("Terminals\n Interrupt Line 7",
                        LIB_PATH "icons/terminal-32.png",
                        EXT_IL_INDEX(IL_TERMINAL),
                        "Terminals", "Terminal", tab);
    return tab;
}

void MachineConfigDialog::registerDeviceClass(const QString& label,
                                              const QString& icon,
                                              unsigned int   devClassIndex,
                                              const QString& devClassName,
                                              const QString& devName,
                                              QWidget*       parent,
                                              bool           selected)
{
    /* FIXME: insert accessible names here */
    QWidget* devfc;

    if (devClassIndex == EXT_IL_INDEX(IL_ETHERNET))
        devfc = new NetworkConfigWidget(parent);
    else
        devfc = new DeviceFileChooser(devClassName, devName, devClassIndex, parent);

    devfc->setAccessibleName(devClassName+" Properties");
    connect(this, SIGNAL(accepted()), devfc, SLOT(Save()));
    devFileChooserStack->addWidget(devfc);

    devClassView->addItem(QIcon(icon), devClassName);
}

void MachineConfigDialog::getROMFileName(int index)
{
    QString title = QString("Select a %1 File").arg(romFileInfo[index].description);

    QString fileName = QFileDialog::getOpenFileName(this, title);
    if (!fileName.isEmpty())
        romFileInfo[index].lineEdit->setText(fileName);
}

void MachineConfigDialog::saveConfigChanges()
{
    /* STATIC: only one core
    config->setNumProcessors(cpuSpinner->value()); */
    config->setClockRate(clockRateSpinner->value());
    config->setRefreshRate(refreshRateSpinner->value());
    config->setRefreshOnPause(!refreshEnabledBox->isChecked());
    config->setRamSize(ramSizeSpinner->value());
    config->setTLBSize(MachineConfig::MIN_TLB_SIZE << tlbSizeList->currentIndex());

    config->setROM(ROM_TYPE_BIOS,
                   QFile::encodeName(romFileInfo[ROM_TYPE_BIOS].lineEdit->text()).constData());
    config->setROM(ROM_TYPE_CORE,
                   QFile::encodeName(romFileInfo[ROM_TYPE_CORE].lineEdit->text()).constData());
    config->setROM(ROM_TYPE_STAB,
                   QFile::encodeName(romFileInfo[ROM_TYPE_STAB].lineEdit->text()).constData());

    /* STATIC: when dynamic loading will be possible remove this!
    config->setLoadCoreEnabled(coreBootCheckBox->isChecked()); */
    config->setStopOnException(stopOnInterruptBox->isChecked());
    config->setSymbolTableASID(stabAsidEdit->getAsid());
}


DeviceFileChooser::DeviceFileChooser(const QString& deviceClassName,
                                     const QString& deviceName,
                                     unsigned int   line,
                                     QWidget*       parent)
    : QWidget(parent),
      il(line),
      deviceName(deviceName)
{
    QSignalMapper* signalMapper = new QSignalMapper(this);

    QGridLayout* grid = new QGridLayout(this);

    QLabel* header = new QLabel(deviceClassName, this);
    QFont font;
    font.setPointSizeF(font.pointSizeF() * 1.5);
    header->setFont(font);
    grid->addWidget(header, 0, 0, 1, 2);

    grid->addWidget(new QLabel("<b>Device File<b>", this), 1, 1);
    grid->addWidget(new QLabel("<b>Enable<b>", this), 1, 3);

    for (unsigned int i = 0; i < N_DEV_PER_IL; i++) {
        QLabel* fileLabel = new QLabel(QString("&%1:").arg(i), this);
        fileNameEdit[i] = new QLineEdit(this);
        fileNameEdit[i]->setAccessibleName(deviceName+QString(" %1 file").arg(i));
        fileNameEdit[i]->setToolTip(fileNameEdit[i]->accessibleName());
        fileLabel->setBuddy(fileNameEdit[i]);
        fileNameEdit[i]->setText(MC_Holder::getInstance()->getConfig()->getDeviceFile(il, i).c_str());
        QPushButton* bt = new QPushButton("Browse...", this);
        bt->setAccessibleName("Browse for "+fileNameEdit[i]->accessibleName());
        connect(bt, SIGNAL(clicked()), signalMapper, SLOT(map()));
        signalMapper->setMapping(bt, (int) i);
        enabledCB[i] = new QCheckBox(this);
        enabledCB[i]->setChecked(MC_Holder::getInstance()->getConfig()->getDeviceEnabled(il, i));
        enabledCB[i]->setAccessibleName("Enable "+deviceName+QString(" %1").arg(i));
        enabledCB[i]->setToolTip(enabledCB[i]->accessibleName());

        grid->addWidget(fileLabel, i + 2, 0);
        grid->addWidget(fileNameEdit[i], i + 2, 1);
        grid->addWidget(bt, i + 2, 2);
        grid->addWidget(enabledCB[i], i + 2, 3, Qt::AlignCenter);
    }

    grid->setColumnMinimumWidth(1, 190);
    grid->setRowStretch(11, 1);

    connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(browseDeviceFile(int)));
}

QString DeviceFileChooser::getDeviceFile(unsigned int devNo)
{
    assert(devNo < N_DEV_PER_IL);
    return fileNameEdit[devNo]->text();
}

bool DeviceFileChooser::IsDeviceEnabled(unsigned int devNo)
{
    assert(devNo < N_DEV_PER_IL);
    return enabledCB[devNo]->isChecked();
}

void DeviceFileChooser::Save()
{
    for (unsigned int devNo = 0; devNo < N_DEV_PER_IL; devNo++) {
        MC_Holder::getInstance()->getConfig()->setDeviceFile(il, devNo, QFile::encodeName(fileNameEdit[devNo]->text()).constData());
        MC_Holder::getInstance()->getConfig()->setDeviceEnabled(il, devNo, enabledCB[devNo]->isChecked());
    }
}

void DeviceFileChooser::browseDeviceFile(int devNo)
{
    QString title = QString("Select %1 device file").arg(deviceName);
    QString fileName = QFileDialog::getOpenFileName(this, title);
    if (!fileName.isNull()) {
        fileNameEdit[devNo]->setText(fileName);
    }
}


NetworkConfigWidget::NetworkConfigWidget(QWidget* parent)
    : QWidget(parent)
{
    const unsigned int il = EXT_IL_INDEX(IL_ETHERNET);
    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* header = new QLabel("Network Adapters", this);
    QFont font;
    font.setPointSizeF(font.pointSizeF() * 1.5);
    header->setFont(font);
    layout->addWidget(header);
    QFrame* separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    layout->addWidget(separator);

    QComboBox* nics = new QComboBox(this);
    nics->setAccessibleName("Select Network Adapter");
    nics->setAccessibleDescription("Select witch Adapter to configure");
    for (unsigned int i = 0; i < N_DEV_PER_IL; i++)
        nics->addItem(QString("Network Adapter %1").arg(i));
    layout->addWidget(nics);

    QStackedLayout* nicConfigStack = new QStackedLayout();
    layout->addLayout(nicConfigStack);

    connect(nics, SIGNAL(currentIndexChanged(int)),
            nicConfigStack, SLOT(setCurrentIndex(int)));

    QSignalMapper* signalMapper = new QSignalMapper(this);
    connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(browseDeviceFile(int)));

    MachineConfig* machineConfigs = MC_Holder::getInstance()->getConfig();

    for (unsigned int i = 0; i < N_DEV_PER_IL; i++) {
        QWidget* widget = new QWidget(this);
        nicConfigStack->addWidget(widget);
        QVBoxLayout* box = new QVBoxLayout(widget);
        box->setContentsMargins(0, 6, 0, 0);

        enabledCB[i] = new QCheckBox("&Enable", widget);
        enabledCB[i]->setAccessibleName(QString("Enable Network Adapter %1").arg(i));
        box->addWidget(enabledCB[i]);

        QWidget* form = new QWidget(widget);
        QGridLayout* grid = new QGridLayout(form);
        box->addWidget(form);
        grid->setContentsMargins(0, 0, 0, 0);

        connect(enabledCB[i], SIGNAL(toggled(bool)), form, SLOT(setEnabled(bool)));
        enabledCB[i]->setChecked(machineConfigs->getDeviceEnabled(il, i));
        form->setEnabled(machineConfigs->getDeviceEnabled(il, i));

        QLabel* fileLabel = new QLabel("Device &File:", form);
        fileEdit[i] = new QLineEdit(form);
        fileEdit[i]->setAccessibleName(QString("Network Adapter %1 file").arg(i));
        fileLabel->setBuddy(fileEdit[i]);
        fileEdit[i]->setText(machineConfigs->getDeviceFile(il, i).c_str());
        grid->addWidget(fileLabel, 1, 0);
        grid->addWidget(fileEdit[i], 1, 1);
        QPushButton* fileBt = new QPushButton("&Browse...", form);
        fileBt->setAccessibleName("Browse for "+fileEdit[i]->accessibleName());
        grid->addWidget(fileBt, 1, 2);
        connect(fileBt, SIGNAL(clicked()), signalMapper, SLOT(map()));
        signalMapper->setMapping(fileBt, (int) i);

        fixedMacId[i] = new QCheckBox("Fixed &MAC address", form);
        fixedMacId[i]->setAccessibleName("Fixed MAC address");
        fixedMacId[i]->setAccessibleDescription(QString("Enable fixed MAC address for Newtork Adapter %1").arg(i));
        grid->addWidget(fixedMacId[i], 2, 0, 1, 3);
        QLabel* macIdLabel = new QLabel("MAC &Address", form);
        grid->addWidget(macIdLabel, 3, 0);
        macIdEdit[i] = new MacIdEdit(form);
        macIdEdit[i]->setAccessibleName(QString("MAC Address for Network Adapter %1").arg(i));
        macIdEdit[i]->setFont(monoLabel::getMonospaceFont());
        macIdLabel->setBuddy(macIdEdit[i]);
        if (machineConfigs->getMACId(i))
            macIdEdit[i]->setMacId(machineConfigs->getMACId(i));
        grid->addWidget(macIdEdit[i], 3, 1, 1, 2);
        connect(fixedMacId[i], SIGNAL(toggled(bool)), macIdLabel, SLOT(setEnabled(bool)));
        connect(fixedMacId[i], SIGNAL(toggled(bool)), macIdEdit[i], SLOT(setEnabled(bool)));
        fixedMacId[i]->setChecked(machineConfigs->getMACId(i) != NULL);
        macIdLabel->setEnabled(machineConfigs->getMACId(i) != NULL);
        macIdEdit[i]->setEnabled(machineConfigs->getMACId(i) != NULL);
    }

    layout->addStretch(1);
}

void NetworkConfigWidget::Save()
{
    const unsigned int il = EXT_IL_INDEX(IL_ETHERNET);

    MachineConfig *machineConfigs = MC_Holder::getInstance()->getConfig();

    for (unsigned int devNo = 0; devNo < N_DEV_PER_IL; devNo++) {
        machineConfigs->setDeviceFile(il, devNo, QFile::encodeName(fileEdit[devNo]->text()).constData());
        machineConfigs->setDeviceEnabled(il, devNo, enabledCB[devNo]->isChecked());
        if (fixedMacId[devNo]->isChecked()) {
            uint8_t macId[6];
            assert(macIdEdit[devNo]->getMacId(macId));
            machineConfigs->setMACId(devNo, macId);
        } else {
            machineConfigs->setMACId(devNo, NULL);
        }
    }
}

void NetworkConfigWidget::browseDeviceFile(int devNo)
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select Device File");
    if (!fileName.isNull())
        fileEdit[devNo]->setText(fileName);
}

#endif //QARM_MACHINE_CONFIG_DIALOG_CC
