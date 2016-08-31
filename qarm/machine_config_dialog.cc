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
#include "qarm/qarmmessagebox.h"
#include "qarm/qarm.h"
#include "services/debug_session.h"

MachineConfigDialog::MachineConfigDialog(MachineConfig* config, QWidget* parent)
    : QDialog(parent),
      config(config),
      restartNeededNotified(false)
{
    setWindowTitle("Machine Configuration");
    setWindowIcon(QIcon(LIB_PATH "icons/config.png"));

    QTabWidget* tabWidget = new QTabWidget(this);
    tabWidget->addTab(createGeneralTab(tabWidget), "&General");
    tabWidget->addTab(createDeviceTab(tabWidget), "&Devices");
    tabWidget->addTab(createAccessibilityTab(tabWidget), "&Accessibility");

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

    /* STATIC: only one core
    layout->addWidget(new QLabel("Processors:"), 1, 1);
    cpuSpinner = new QSpinBox();
    cpuSpinner->setMinimum(MachineConfig::MIN_CPUS);
    cpuSpinner->setMaximum(MachineConfig::MAX_CPUS);
    cpuSpinner->setValue(config->getNumProcessors());
    layout->addWidget(cpuSpinner, 1, 3);*/

    clockRateSpinner = new QSpinBox(tabWidget);
    clockRateSpinner->setAccessibleName("Default Clock Rate (MHz):");
    clockRateSpinner->setAccessibleDescription("This will change the relative time taken by devices to operate");
    clockRateSpinner->setToolTip(clockRateSpinner->accessibleDescription());
    clockRateSpinner->setMinimum(MachineConfig::MIN_CLOCK_RATE);
    clockRateSpinner->setMaximum(MachineConfig::MAX_CLOCK_RATE);
    clockRateSpinner->setValue(config->getClockRate());

    refreshRateSpinner = new QSpinBox(tabWidget);
    refreshRateSpinner->setAccessibleName("GUI Refresh Rate");
    refreshRateSpinner->setAccessibleDescription("Number of CPU cycles between each refresh");
    refreshRateSpinner->setToolTip(refreshRateSpinner->accessibleDescription());
    refreshRateSpinner->setMinimum(MachineConfig::MIN_REFRESH_RATE);
    refreshRateSpinner->setMaximum(MachineConfig::MAX_REFRESH_RATE);
    refreshRateSpinner->setValue(config->getRefreshRate());

    refreshEnabledBox = new QCheckBox(tabWidget);
    refreshEnabledBox->setAccessibleName("Enable constant refresh");
    refreshEnabledBox->setAccessibleDescription("If selected, each GUI component gets refreshed each Refresh Rate cycles, else refresh will happen on execution stop");
    refreshEnabledBox->setToolTip(refreshEnabledBox->accessibleDescription());
    refreshEnabledBox->setText(refreshEnabledBox->accessibleName());
    refreshEnabledBox->setChecked(!config->getRefreshOnPause());
    if(!refreshEnabledBox->isChecked())
        refreshRateSpinner->setEnabled(false);

    connect(refreshEnabledBox, SIGNAL(clicked(bool)), refreshRateSpinner, SLOT(setEnabled(bool)));

    ramSizeSpinner = new QSpinBox(tabWidget);
    ramSizeSpinner->setAccessibleName("Ram Size (Frames)");
    ramSizeSpinner->setAccessibleDescription("Total size of accessible RAM expressed in frames");
    ramSizeSpinner->setToolTip(ramSizeSpinner->accessibleDescription());
    ramSizeSpinner->setMinimum(MachineConfig::MIN_RAM);
    ramSizeSpinner->setMaximum(MachineConfig::MAX_RAM);
    ramSizeSpinner->setValue(config->getRamSize());

    ramSizeLabel = new QLabel(tabWidget);
    ramSizeLabel->setAccessibleName("Ram Size (Bytes)");
    ramSizeLabel->setAccessibleDescription("Approximated ram size expressed in bytes");
    ramSizeLabel->setToolTip(ramSizeLabel->accessibleDescription());
    updateRamSize(ramSizeSpinner->value());

    connect(ramSizeSpinner, SIGNAL(valueChanged(int)), this, SLOT(updateRamSize(int)));

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

    stopOnInterruptBox = new QCheckBox("Pause execution on Exception", tabWidget);
    stopOnInterruptBox->setAccessibleName("Pause execution on Exception");
    stopOnInterruptBox->setChecked(config->getStopOnException());

    QSignalMapper* fileChooserMapper = new QSignalMapper(this);
    connect(fileChooserMapper, SIGNAL(mapped(int)), this, SLOT(getROMFileName(int)));
    QPushButton *BIOSFileChooserButton, *coreFileChooserButton;

    romFileInfo[ROM_TYPE_BIOS].description = "Execution ROM";
    romFileInfo[ROM_TYPE_BIOS].lineEdit = new QLineEdit(tabWidget);
    romFileInfo[ROM_TYPE_BIOS].lineEdit->setAccessibleName("Execution ROM");
    romFileInfo[ROM_TYPE_BIOS].lineEdit->setAccessibleDescription("Path to Execution ROM file");
    romFileInfo[ROM_TYPE_BIOS].lineEdit->setToolTip(romFileInfo[ROM_TYPE_BIOS].lineEdit->accessibleDescription());
    romFileInfo[ROM_TYPE_BIOS].lineEdit->setText(config->getROM(ROM_TYPE_BIOS).c_str());
    BIOSFileChooserButton = new QPushButton("Browse...", tabWidget);
    BIOSFileChooserButton->setAccessibleName("Browse for Execution ROM");
    connect(BIOSFileChooserButton, SIGNAL(clicked()), fileChooserMapper, SLOT(map()));
    fileChooserMapper->setMapping(BIOSFileChooserButton, ROM_TYPE_BIOS);

    /* STATIC: when dynamic loading will be possible remove this!
    coreBootCheckBox = new QCheckBox("Load core file");
    coreBootCheckBox->setChecked(config->isLoadCoreEnabled());

    coreBootCheckBox->setEnabled(false);
    layout->addWidget(coreBootCheckBox, 11, 1, 1, 3);*/

    romFileInfo[ROM_TYPE_CORE].description = "Core";
    romFileInfo[ROM_TYPE_CORE].lineEdit = new QLineEdit(tabWidget);
    romFileInfo[ROM_TYPE_CORE].lineEdit->setAccessibleName("Core file");
    romFileInfo[ROM_TYPE_CORE].lineEdit->setAccessibleDescription("Path to Core file");
    romFileInfo[ROM_TYPE_CORE].lineEdit->setToolTip(romFileInfo[ROM_TYPE_CORE].lineEdit->accessibleDescription());
    romFileInfo[ROM_TYPE_CORE].lineEdit->setText(config->getROM(ROM_TYPE_CORE).c_str());
    coreFileChooserButton = new QPushButton("Browse...", tabWidget);
    coreFileChooserButton->setAccessibleName("Browse for Core file");
    connect(coreFileChooserButton, SIGNAL(clicked()), fileChooserMapper, SLOT(map()));
    fileChooserMapper->setMapping(coreFileChooserButton, ROM_TYPE_CORE);

    romFileInfo[ROM_TYPE_STAB].description = "External Symbol Table";
    romFileInfo[ROM_TYPE_STAB].lineEdit = new QLineEdit(tabWidget);
    romFileInfo[ROM_TYPE_STAB].lineEdit->setAccessibleName("External Symbol Table");
    romFileInfo[ROM_TYPE_STAB].lineEdit->setAccessibleDescription("Path to external executable to read Symbol Table from");
    romFileInfo[ROM_TYPE_STAB].lineEdit->setToolTip(romFileInfo[ROM_TYPE_STAB].lineEdit->accessibleDescription());
    romFileInfo[ROM_TYPE_STAB].lineEdit->setText(config->getROM(ROM_TYPE_STAB).c_str());
    stabFileChooserButton = new QPushButton("Browse...", tabWidget);
    stabFileChooserButton->setAccessibleName("Browse for external executable file");
    connect(stabFileChooserButton, SIGNAL(clicked()), fileChooserMapper, SLOT(map()));
    connect(romFileInfo[ROM_TYPE_STAB].lineEdit, SIGNAL(textChanged(QString)), this, SLOT(alertNeedsRestart()));
    fileChooserMapper->setMapping(stabFileChooserButton, ROM_TYPE_STAB);

    externalStabBox = new QCheckBox("External Symbol Table", tabWidget);
    connect(externalStabBox, SIGNAL(toggled(bool)), this, SLOT(toggleExternalStab(bool)));
    externalStabBox->setAccessibleName("Enable external Symbol Table");
    externalStabBox->setChecked(config->getExternalStab());
    toggleExternalStab(externalStabBox->isChecked());
    connect(externalStabBox, SIGNAL(toggled(bool)), this, SLOT(alertNeedsRestart()));

    stabAsidEdit = new AsidLineEdit(tabWidget);
    stabAsidEdit->setAccessibleName("Symbol Table ASID");
    stabAsidEdit->setAccessibleDescription("Default Symbol Table ASID");
    stabAsidEdit->setToolTip(stabAsidEdit->accessibleDescription());
    stabAsidEdit->setMaximumWidth(100);
    stabAsidEdit->setAsid(config->getSymbolTableASID());

    if(config->getAccessibleMode()){ //if accessible mode layout should be flatten
        QVBoxLayout *layout = new QVBoxLayout(tabWidget);
        layout->setContentsMargins(5, 5, 5, 5);
        layout->addWidget(new QLabel("<b>Hardware</b>", tabWidget));

        QHBoxLayout *tempHL = new QHBoxLayout();
        tempHL->addWidget(new QLabel("Default Clock Rate (MHz):", tabWidget));
        tempHL->addWidget(clockRateSpinner);
        layout->addLayout(tempHL);

        tempHL = new QHBoxLayout();
        tempHL->addWidget(new QLabel("GUI Refresh Rate:", tabWidget));
        tempHL->addWidget(refreshRateSpinner);
        layout->addLayout(tempHL);

        layout->addWidget(refreshEnabledBox);

        tempHL = new QHBoxLayout();
        tempHL->addWidget(new QLabel("RAM Size (Frames):", tabWidget));
        tempHL->addWidget(ramSizeSpinner);
        layout->addLayout(tempHL);

        layout->addWidget(ramSizeLabel);

        tempHL = new QHBoxLayout();
        tempHL->addWidget(new QLabel("TLB Size (Entries):", tabWidget));
        tempHL->addWidget(tlbSizeList);
        layout->addLayout(tempHL);

        layout->addWidget(stopOnInterruptBox);

        layout->addWidget(new QLabel("<b>BIOS</b>", tabWidget));

        tempHL = new QHBoxLayout();
        tempHL->addWidget(new QLabel("Execution ROM:", tabWidget));
        tempHL->addWidget(romFileInfo[ROM_TYPE_BIOS].lineEdit);
        tempHL->addWidget(BIOSFileChooserButton);
        layout->addLayout(tempHL);

        layout->addWidget(new QLabel("<b>Boot</b>", tabWidget));

        tempHL = new QHBoxLayout();
        tempHL->addWidget(new QLabel("Core file:", tabWidget));
        tempHL->addWidget(romFileInfo[ROM_TYPE_CORE].lineEdit);
        tempHL->addWidget(coreFileChooserButton);
        layout->addLayout(tempHL);

        layout->addWidget(new QLabel("<b>Debugging Support</b>", tabWidget));

        tempHL = new QHBoxLayout();
        tempHL->addWidget(new QLabel("Symbol Table ASID:", tabWidget));
        tempHL->addWidget(stabAsidEdit);
        layout->addLayout(tempHL);

        tempHL = new QHBoxLayout();
        tempHL->addWidget(new QLabel("Enable External Symbol Table:", tabWidget));
        tempHL->addWidget(externalStabBox);
        layout->addLayout(tempHL);

        tempHL = new QHBoxLayout();
        tempHL->addWidget(new QLabel("External Symbol Table File:", tabWidget));
        tempHL->addWidget(romFileInfo[ROM_TYPE_STAB].lineEdit);
        tempHL->addWidget(stabFileChooserButton);
        layout->addLayout(tempHL);
    } else {
        QGridLayout *layout = new QGridLayout(tabWidget);
        layout->setContentsMargins(5, 5, 5, 5);
        layout->addWidget(new QLabel("<b>Hardware</b>", tabWidget), 0, 0, 1, 3);

        layout->addWidget(new QLabel("Default Clock Rate (MHz):", tabWidget), 1, 1);
        layout->addWidget(clockRateSpinner, 1, 2);

        layout->addWidget(new QLabel("GUI Refresh Rate:", tabWidget), 2, 1);
        layout->addWidget(refreshRateSpinner, 2, 2);
        layout->addWidget(refreshEnabledBox, 2, 3);

        layout->addWidget(new QLabel("RAM Size (Frames):", tabWidget), 3, 1);
        layout->addWidget(ramSizeSpinner, 3, 2);
        layout->addWidget(ramSizeLabel, 3, 3);

        layout->addWidget(new QLabel("TLB Size (Entries):", tabWidget), 4, 1);
        layout->addWidget(tlbSizeList, 4, 2);

        layout->addWidget(stopOnInterruptBox, 5, 1, 1, 3);

        layout->addWidget(new QLabel("<b>BIOS</b>", tabWidget), 6, 0, 1, 3);

        layout->addWidget(new QLabel("Execution ROM:", tabWidget), 7, 1);
        layout->addWidget(romFileInfo[ROM_TYPE_BIOS].lineEdit, 7, 2, 1, 2);
        layout->addWidget(BIOSFileChooserButton, 7, 4);

        layout->addWidget(new QLabel("<b>Boot</b>", tabWidget), 8, 0, 1, 3);

        layout->addWidget(new QLabel("Core file:", tabWidget), 9, 1);
        layout->addWidget(romFileInfo[ROM_TYPE_CORE].lineEdit, 9, 2, 1, 2);
        layout->addWidget(coreFileChooserButton, 9, 4);

        layout->addWidget(new QLabel("<b>Debugging Support</b>", tabWidget), 10, 0, 1, 3);

        layout->addWidget(new QLabel("Symbol Table ASID:", tabWidget), 11, 1);
        layout->addWidget(stabAsidEdit, 11, 2);
        layout->addWidget(externalStabBox, 11, 3);

        layout->addWidget(new QLabel("External Symbol Table:", tabWidget), 12, 1);
        layout->addWidget(romFileInfo[ROM_TYPE_STAB].lineEdit, 12, 2, 1, 2);
        layout->addWidget(stabFileChooserButton, 12, 4);


    }

    return tabWidget;
}

QWidget* MachineConfigDialog::createAccessibilityTab(QWidget *parent){
    QWidget* tab = new QWidget(parent);
    tab->setAccessibleName("Accessibility Settings");
    QVBoxLayout* tabLayout = new QVBoxLayout(tab);

    tabLayout->setContentsMargins(5, 5, 5, 5);

    tabLayout->addWidget(new QLabel("<b>Accessibility Settings</b>", tab));

    accessibleModeCheckBox = new QCheckBox("Enable Increased Accessibility \n(Restart Needed)", tab);
    accessibleModeCheckBox->setChecked(config->getAccessibleMode());
    tabLayout->addWidget(accessibleModeCheckBox);

    tabLayout->addStretch();

    return tab;
}

QWidget* MachineConfigDialog::createDeviceTab(QWidget *parent)
{
    static const int TAB_MARGIN_TOP = 5;
    static const int TAB_MARGIN_BOTTOM = 5;
    static const int TAB_MARGIN_LEFT = 5;
    static const int TAB_MARGIN_RIGHT = 5;

    QWidget* tab = new QWidget(parent);
    tab->setAccessibleName("Devices Settings");
    QVBoxLayout* tabLayout = new QVBoxLayout(tab);

    tabLayout->setContentsMargins(TAB_MARGIN_TOP,
                                  TAB_MARGIN_BOTTOM,
                                  TAB_MARGIN_LEFT,
                                  TAB_MARGIN_RIGHT);

    devClassView = new QComboBox(tab);
    devClassView->setAccessibleName("Device Type");

    tabLayout->addWidget(new QLabel("<b>Devices settings</b>", tab));
    tabLayout->addWidget(devClassView);

    devFileChooserStack = new QStackedLayout();
    tabLayout->addLayout(devFileChooserStack);

    connect(devClassView, SIGNAL(currentIndexChanged(int)), devFileChooserStack, SLOT(setCurrentIndex(int)));

    registerDeviceClass("Tapes (Interrupt Line 4)",
                        LIB_PATH "icons/tape-32.png",
                        EXT_IL_INDEX(IL_TAPE),
                        "Tapes", "Tape", tab);

    registerDeviceClass("Disks (Interrupt Line 3)",
                        LIB_PATH "icons/disk-32.png",
                        EXT_IL_INDEX(IL_DISK),
                        "Disks", "Disk",
                        tab);

    registerDeviceClass("Terminals (Interrupt Line 7)",
                        LIB_PATH "icons/terminal-32.png",
                        EXT_IL_INDEX(IL_TERMINAL),
                        "Terminals", "Terminal", tab);

    registerDeviceClass("Printers (Interrupt Line 6)",
                        LIB_PATH "icons/printer-32.png",
                        EXT_IL_INDEX(IL_PRINTER),
                        "Printers", "Printer", tab);

    registerDeviceClass("Network (Interrupt Line 5)",
                        LIB_PATH "icons/network-32.png",
                        EXT_IL_INDEX(IL_ETHERNET),
                        "Network Interfaces", "Net", tab);


    return tab;
}

void MachineConfigDialog::updateRamSize(int frames){
    int kilos = frames*4;
    float megs = kilos / 1024.0;
    float gigs = megs / 1024.0;
    int sel = (gigs >= 0.7 ? 2 : (megs >= 1 ? 1 : 0));
    QString text;
    switch(sel){
        case 0: //KB
            text = QString::number(kilos) + " KB";
            break;
        case 1: //MB
            text = QString::number(megs, 'g', 4) + " MB";
            break;
        case 2: //GB
            text = QString::number(gigs, 'g', 4) + " GB";
            break;
    }
    ramSizeLabel->setText(text);
}

void MachineConfigDialog::registerDeviceClass(const QString& label,
                                              const QString& icon,
                                              unsigned int   devClassIndex,
                                              const QString& devClassName,
                                              const QString& devName,
                                              QWidget*       parent)
{
    /* FIXME: insert accessible names here */
    QWidget* devfc;

    if (devClassIndex == EXT_IL_INDEX(IL_ETHERNET))
        devfc = new NetworkConfigWidget(label, parent);
    else
        devfc = new DeviceFileChooser(label, devName, devClassIndex, parent);

    devfc->setAccessibleName(devClassName+" Properties");
    connect(this, SIGNAL(accepted()), devfc, SLOT(Save()));
    devFileChooserStack->addWidget(devfc);

    devClassView->addItem(QIcon(icon), devClassName);
}

void MachineConfigDialog::alertNeedsRestart(){
    if(DebuggerHolder::getInstance()->getDebugSession()->isRunning() && !restartNeededNotified){
        QarmMessageBox *warning = new QarmMessageBox(QarmMessageBox::WARNING, "Warning",
                                                     "Changes may need a restart to take effect.", this);
        warning->show();
        restartNeededNotified = true;
    }
}

void MachineConfigDialog::toggleExternalStab(bool state){
    romFileInfo[ROM_TYPE_STAB].lineEdit->setEnabled(state);
    stabFileChooserButton->setEnabled(state);
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
    config->setExternalStab(externalStabBox->isChecked());

    config->setAccessibleMode(accessibleModeCheckBox->isChecked());
    config->getMainWidget()->disableMainbar(true, false);
}


DeviceFileChooser::DeviceFileChooser(const QString& label,
                                     const QString& deviceName,
                                     unsigned int   line,
                                     QWidget*       parent)
    : QWidget(parent),
      il(line),
      deviceName(deviceName)
{
    QSignalMapper* signalMapper = new QSignalMapper(this);

    QGridLayout* grid = new QGridLayout(this);

    QLabel* header = new QLabel(label, this);
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


NetworkConfigWidget::NetworkConfigWidget(const QString& label, QWidget* parent)
    : QWidget(parent)
{
    const unsigned int il = EXT_IL_INDEX(IL_ETHERNET);
    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* header = new QLabel(label, this);
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
