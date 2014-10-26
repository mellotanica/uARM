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

#ifndef QARM_QARM_CC
#define QARM_QARM_CC

#include "qarm/qarm.h"
#include "armProc/aout.h"
#include "qarm/machine_config_dialog.h"
#include "services/error.h"

#include <QFileDialog>
#include <QFile>
#include <QScrollArea>
#include <QMetaObject>
#include <QIcon>

qarm::qarm(QApplication *app):
    application(app)
{
    // INFO: machine config init
    std::string error;
    QDir *defaultPath = new QDir(QDir::homePath()+"/"+DEFAULT_CONFIG_PATH);
    if(!defaultPath->exists())
        if(!defaultPath->mkdir(defaultPath->absolutePath())){   //config folder not accessible..
            QMessageBox::critical(this, "Fatal", "Cannot create .uarm folder in home directory.\nCheck HOME environment variable.");
            app->exit();
        }

    DebugSession *debugger;
    if((debugger = DebuggerHolder::getInstance()->getDebugSession()) == NULL){
        debugger = new DebugSession(application, this);
        DebuggerHolder::getInstance()->setDebugSession(debugger);
    }

    std::string defaultFName = defaultPath->absolutePath().toStdString()+"/"+DEFAULT_CONFIG_FILE;
    MC_Holder::getInstance()->setConfig(MachineConfig::LoadFromFile(defaultFName, error, app));
    if(MC_Holder::getInstance()->getConfig() == NULL)
        MC_Holder::getInstance()->setConfig(MachineConfig::Create(defaultFName, app));

    mac = new machine(debugger->getBreakpoints(),debugger->getSuspects(),debugger->getTracepoints());

    setWindowTitle("uARM");
    setWindowIcon(QIcon(":/icons/window_default-48.png"));

    mainWidget = new QWidget;
    toolbar = new mainBar;
    display = new procDisplay(this);

    toolbar->setSpeed(IPSMAX);

    centralLayout = new QVBoxLayout;

    centralLayout->addWidget(new QFLine(false));
    centralLayout->addWidget(toolbar);
    centralLayout->addWidget(new QFLine(false));
    centralLayout->addWidget(display);

    mainWidget->setLayout(centralLayout);

    clock = new QTimer(this);

    bpWindow = new breakpoint_window(mac, this);

    connect(toolbar, SIGNAL(play(int)), this, SLOT(start(int)));
    connect(toolbar, SIGNAL(speedChanged(int)), this, SLOT(speedChanged(int)));
    connect(toolbar, SIGNAL(pause()), this, SLOT(stop()));
    connect(toolbar, SIGNAL(reset()), this, SLOT(softReset()));
    connect(toolbar, SIGNAL(showRam()), this, SLOT(showRam()));
    connect(toolbar, SIGNAL(step()), this, SLOT(step()));
    connect(toolbar, SIGNAL(showConfig()), this, SLOT(showConfigDialog()));
    connect(toolbar, SIGNAL(showTerminal(uint)), this, SLOT(showTerminal(uint)));
    connect(this, SIGNAL(setTerminalEnabled(uint,bool)), toolbar, SLOT(setTerminalEnabled(uint,bool)));

    connect(toolbar, SIGNAL(showBPW()), bpWindow, SLOT(show()));
    connect(toolbar, SIGNAL(hideBPW()), bpWindow, SLOT(hide()));
    connect(bpWindow, SIGNAL(hiding()), toolbar, SLOT(uncheckBPB()));
    connect(this, SIGNAL(resetMachine()), bpWindow, SLOT(reset()));

    connect(clock, SIGNAL(timeout()), this, SLOT(step()));

    connect(this, SIGNAL(resetMachine()), mac, SLOT(reset()));
    connect(this, SIGNAL(resetMachine()), this, SIGNAL(resetDisplay()));
    connect(mac, SIGNAL(dataReady(Word*,Word*,Word*,Word,Word,Word,QString)), display, SLOT(updateVals(Word*,Word*,Word*,Word,Word,Word,QString)));
    connect(this, SIGNAL(resetDisplay()), display, SLOT(reset()));

    connect(this, SIGNAL(stopSig()), clock, SLOT(stop()));
    connect(this, SIGNAL(stopSig()), toolbar, SLOT(stop()));

    connect(mac, SIGNAL(updateStatus(QString)), toolbar, SLOT(updateStatus(QString)));

    connect(this, SIGNAL(resetMachine()), debugger, SLOT(resetSymbolTable()));
    connect(debugger, SIGNAL(stabUpdated()), bpWindow, SLOT(updateContent()));
    connect(this, SIGNAL(resumeExec()), debugger, SIGNAL(MachineRan()));

    setCentralWidget(mainWidget);
}

void qarm::softReset(){
    stop();
    initialized = false;
    emit resetMachine();
    toolbar->setSpeed(IPSMAX);
    doReset = false;
    initialize();
}

bool qarm::initialize(){
    initialized = true;
    initialized &= openBIOS();
    initialized &= openRAM();
    if(initialized){
        for (unsigned int i = 0; i < N_DEV_PER_IL; ++i) {
            const Device* d = mac->getBus()->getDev(EXT_IL_INDEX(IL_TERMINAL), i);
            emit setTerminalEnabled(i, d->Type() == TERMDEV);
        }
    }
    return initialized;
}

void qarm::stop(){
    emit stopSig();
    resuming = true;
    mac->setUIupdate(0);
}

void qarm::step(){
    if(doReset){
        emit resetMachine();
        doReset = false;
    }
    if(!initialized){
        if(!initialize()){
            emit stop();
            return;
        }
    }
    if(resuming){
        emit resumeExec();
        mac->clearCause();
        resuming = false;
    }
    mac->step();
    if((mac->getBus()->getProcessor(0)->exceptionRaised() && MC_Holder::getInstance()->getConfig()->getStopOnException()) ||   //STATIC: refer to processor n if multiprocessor is implemented
            mac->getBus()->getProcessor(0)->getStatus() == PS_HALTED ||
            mac->isStopRequested()){
        mac->refreshData(true);
        emit stop();
        return;
    }
}

void qarm::start(int speed){
    int time;
    if(speed <= IPSTRESH) {
        time = 1000 / speed;
        mac->setUIupdate(0);
    } else {
        time = 0;
        mac->setUIupdate(MC_Holder::getInstance()->getConfig()->getRefreshRate());
    }
    clock->start(time);
}

void qarm::speedChanged(int speed){
    if(clock->isActive()){
        clock->stop();
        start(speed);
    }
}

void qarm::showRam(){
    if(initialized){
        ramView *ramWindow = new ramView(mac, this);
        connect(this, SIGNAL(resetMachine()), ramWindow, SLOT(update()));
        connect(mac, SIGNAL(dataReady(Word*,Word*,Word*,Word,Word,Word,QString)), ramWindow, SLOT(update()));
        ramWindow->show();
    } else {
        QMessageBox::warning(this, "Warning", "Machine not initialized,\ncannot display memory contents.", QMessageBox::Ok);
    }
}

void qarm::selectCore(){
    if(dataLoaded){
        QMessageBox::StandardButton reply = QMessageBox::question(this,"Caution",
                                                                  "Program File already loaded..\nReset machine and load new Program?",
                                                                  QMessageBox::Yes|QMessageBox::No);
        if(reply == QMessageBox::No)
            return;
        else {
            initialized = false;
            doReset = true;
        }
    }
    QString fileName = QFileDialog::getOpenFileName(this, "Open Program File", "", "Program Files (*.core.uarm);;Binary Files (*.bin);;All Files (*.*)");
    if(fileName != ""){
        coreF = fileName;
        dataLoaded = true;
    }
}

void qarm::selectBios(){
    if(biosLoaded){
        QMessageBox::StandardButton reply = QMessageBox::question(this,"Caution",
                                                                  "BIOS ROM already loaded..\nReset machine and load new BIOS?",
                                                                  QMessageBox::Yes|QMessageBox::No);
        if(reply == QMessageBox::No)
            return;
        else {
            initialized = false;
            doReset = true;
        }
    }
    QString fileName = QFileDialog::getOpenFileName(this, "Open BIOS File", "", "BIOS Files (*.rom.uarm);;Binary Files (*.bin);;All Files (*.*)");
    if(fileName != ""){
        biosF = fileName;
        biosLoaded = true;
    }
}

bool qarm::openRAM(){
    coreF = QString::fromStdString(MC_Holder::getInstance()->getConfig()->getROM(ROM_TYPE_CORE));
    if(coreF != ""){
        QFile f (coreF);
        if(!f.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, "Error", "Could not open Core file");
            return false;
        }
        QDataStream in(&f);
        ramMemory *ram = mac->getBus()->getRam();
        if(ram != NULL){
            Word len = (Word) f.size();
            char *buffer = new char[len];
            int sz = in.readRawData(buffer, len);
            if(sz <= 0 || (buffer[0] | buffer[1]<<8 | buffer[2]<<16 | buffer[3]<<24) != COREFILEID){
                QMessageBox::critical(this, "Error", "Irregular Core file");
                return false;
            }
            sz -= 4;
            if(sz <= 0 || !mac->getBus()->loadRAM(buffer+4, (Word) sz, true)){
                QMessageBox::critical(this, "Error", "Problems while loading Core file");
                return false;
            }
            delete [] buffer;
        }
        f.close();
        //mac->refreshData();
    }
    return true;
}

bool qarm::openBIOS(){
    biosF = QString::fromStdString(MC_Holder::getInstance()->getConfig()->getROM(ROM_TYPE_BIOS));
    if(biosF != ""){
        QFile f (biosF);
        if(!f.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, "Error", "Could not open BIOS file");
            return false;
        }
        QDataStream in(&f);
        Word len = (Word) f.size();
        char *buffer = new char[len];
        Word sz = in.readRawData(buffer, len);
        if(sz <= 0 || (buffer[0] | buffer[1]<<8 | buffer[2]<<16 | buffer[3]<<24) != BIOSFILEID){
            QMessageBox::critical(this, "Error", "Irregular BIOS file");
            return false;
        }
        sz -= 8;
        if(sz <= 0 || !mac->getBus()->loadBIOS(buffer+8, (Word) sz)){
            QMessageBox::critical(this, "Error", "Problems while flashing BIOS ROM");
            return false;
        }
        delete [] buffer;
        f.close();
    }
    return true;
}

void qarm::showConfigDialog(){
    assert(MC_Holder::getInstance()->getConfig());

    MachineConfigDialog dialog(MC_Holder::getInstance()->getConfig(), this);
    if (dialog.exec() == QDialog::Accepted) {
        try {
            MC_Holder::getInstance()->getConfig()->Save();
        } catch (FileError& e) {
            QMessageBox::critical(this, QString("%1: Error").arg(application->applicationName()), e.what());
            return;
        }
        // EDIT: no config view for now..
        //configView->Update();
    }
}

void qarm::showTerminal(unsigned int devNo)
{
    if (terminalWindows[devNo].isNull()) {
        terminalWindows[devNo] = new TerminalWindow(devNo, this);
        terminalWindows[devNo]->setAttribute(Qt::WA_QuitOnClose, false);
        terminalWindows[devNo]->setAttribute(Qt::WA_DeleteOnClose);
        terminalWindows[devNo]->show();
    } else {
        terminalWindows[devNo]->activateWindow();
        terminalWindows[devNo]->raise();
    }
}

void qarm::closeEvent(QCloseEvent* event)
{
    /*STATIC: only one core, actually no processor windows
    for (unsigned int i = 0; i < MachineConfig::MAX_CPUS; i++)
        if (cpuWindows[i])
            cpuWindows[i]->close();*/

    for (unsigned int i = 0; i < N_DEV_PER_IL; i++)
        if (terminalWindows[i])
            terminalWindows[i]->close();

    /*EDIT: default size at next start...
    Appl()->settings.setValue("MonitorWindow/geometry", saveGeometry());
    Appl()->settings.setValue("MonitorWindow/ShowStopMask", viewStopMaskAction->isChecked());
    */
    event->accept();
}

void qarm::onMachineHalted()
{
    /*EDIT: debug and multicore aren't still here
    cpuListModel.reset();
    suspectListModel.reset();
    deviceTreeModel.reset();

    for (unsigned int i = 0; i < MachineConfig::MAX_CPUS; i++)
        if (cpuWindows[i])
            cpuWindows[i]->close();*/

    for (unsigned int i = 0; i < N_DEV_PER_IL; i++)
        if (terminalWindows[i])
            terminalWindows[i]->close();

    /*EDIT: we don't have anything of this...
    tabWidget->setTabEnabled(TAB_INDEX_CPU, false);
    tabWidget->setTabEnabled(TAB_INDEX_MEMORY, false);
    tabWidget->setTabEnabled(TAB_INDEX_DEVICES, false);

    for (unsigned int i = 0; i < N_DEV_PER_IL; ++i)
        showTerminalActions[i]->setEnabled(false);

    for (unsigned int i = 0; i < MachineConfig::MAX_CPUS; ++i)
        showCpuWindowActions[i]->setEnabled(false);

    editConfigAction->setEnabled(true);*/
}

#endif //QARM_QARM_CC
