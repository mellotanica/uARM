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

#include "qarm/qarmmessagebox.h"

#include <QFileDialog>
#include <QFile>
#include <QStandardPaths>
#include <QScrollArea>
#include <QMetaObject>
#include <QIcon>
#include <QWindow>

qarm::qarm(QApplication *app, QFile *confFile):
    application(app)
{
    // INFO: machine config init
    std::string error;
    std::string configFilePath;

    if(confFile == NULL){
    	QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    	if(configDir.isEmpty()){
        	configDir = QDir::homePath()+"/."+DEFAULT_CONFIG_PATH;
    	} else {
        	configDir.append(QString("/").append(DEFAULT_CONFIG_PATH));
    	}
    	QDir *defaultPath = new QDir(configDir);
   	if(!defaultPath->exists())
        	if(!defaultPath->mkdir(defaultPath->absolutePath())){   //config folder not accessible..
        	    QarmMessageBox *error = new QarmMessageBox(QarmMessageBox::CRITICAL, "Fatal", "Cannot create uarm folder in config directory!", this);
        	    error->show();
        	    app->exit();
        	}
	configFilePath = defaultPath->absolutePath().toStdString()+"/"+DEFAULT_CONFIG_FILE;
    } else {
	configFilePath = confFile->fileName().toStdString();
    }

    DebugSession *debugger;
    if((debugger = DebuggerHolder::getInstance()->getDebugSession()) == NULL){
        debugger = new DebugSession(application, this);
        DebuggerHolder::getInstance()->setDebugSession(debugger);
    }

    MC_Holder::getInstance()->setConfig(MachineConfig::LoadFromFile(configFilePath, error, app, this));
    if(MC_Holder::getInstance()->getConfig() == NULL)
        MC_Holder::getInstance()->setConfig(MachineConfig::Create(configFilePath, QDir::homePath().toStdString(), app, this));

    closeSc = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this, NULL, NULL, Qt::ApplicationShortcut);
    connect(closeSc, SIGNAL(activated()), this, SLOT(closeFWindow()));

    mac = new machine(debugger->getBreakpoints(),debugger->getSuspects(),debugger->getTracepoints());

    setWindowTitle("uARM");
    setWindowIcon(QIcon(LIB_PATH "icons/window_default-48.png"));

    mainWidget = new QWidget(this);
    toolbar = new mainBar(mainWidget);
    display = new procDisplay(mainWidget);

    toolbar->setSpeed(IPSMAX);

    centralLayout = new QVBoxLayout();

    centralLayout->addWidget(new QFLine(false));
    centralLayout->addWidget(toolbar);
    centralLayout->addWidget(new QFLine(false));
    centralLayout->addWidget(display);

    mainWidget->setLayout(centralLayout);

    clock = new QTimer(this);

    bpWindow = new breakpoint_window(mac, this);
    tlbWindow = new tlb_window(mac, this);

    connect(toolbar, SIGNAL(play(int)), this, SLOT(start(int)));
    connect(toolbar, SIGNAL(speedChanged(int)), this, SLOT(speedChanged(int)));
    connect(toolbar, SIGNAL(pause()), this, SLOT(pause()));
    connect(toolbar, SIGNAL(reset()), this, SLOT(softReset()));
    connect(toolbar, SIGNAL(powerOn()), this, SLOT(powerOn()));
    connect(toolbar, SIGNAL(showRam()), this, SLOT(showRam()));
    connect(toolbar, SIGNAL(step()), this, SLOT(step()));
    connect(toolbar, SIGNAL(showConfig()), this, SLOT(showConfigDialog()));
    connect(toolbar, SIGNAL(showTerminal(uint)), this, SLOT(showTerminal(uint)));
    connect(this, SIGNAL(setTerminalEnabled(uint,bool)), toolbar, SLOT(setTerminalEnabled(uint,bool)));
    connect(this, SIGNAL(poweredOn()), toolbar, SLOT(doPowerOn()));
    connect(this, SIGNAL(poweredOff()), toolbar, SLOT(doPowerOff()));

    connect(toolbar, SIGNAL(showBPW()), bpWindow, SLOT(show()));
    connect(toolbar, SIGNAL(hideBPW()), bpWindow, SLOT(hide()));
    connect(bpWindow, SIGNAL(hiding()), toolbar, SLOT(uncheckBPB()));
    connect(this, SIGNAL(resetMachine()), bpWindow, SLOT(reset()));

    connect(toolbar, SIGNAL(showTLB()), tlbWindow, SLOT(show()));
    connect(toolbar, SIGNAL(hideTLB()), tlbWindow, SLOT(hide()));
    connect(tlbWindow, SIGNAL(hiding()), toolbar, SLOT(uncheckTLB()));

    connect(clock, SIGNAL(timeout()), this, SLOT(step()));

    connect(this, SIGNAL(resetMachine()), mac, SLOT(reset()));
    connect(this, SIGNAL(resetMachine()), this, SIGNAL(resetDisplay()));
    if(MC_Holder::getInstance()->getConfig()->getAccessibleMode()) {
        connect(mac, SIGNAL(dataReady(Word*,Word*,Word*,Word,Word,Word,QString)), display, SLOT(updateTexts(Word*,Word*,Word*,Word,Word,Word,QString)));
        connect(this, SIGNAL(resetDisplay()), display, SLOT(resetTexts()));
    } else {
        connect(mac, SIGNAL(dataReady(Word*,Word*,Word*,Word,Word,Word,QString)), display, SLOT(updateLabels(Word*,Word*,Word*,Word,Word,Word,QString)));
        connect(this, SIGNAL(resetDisplay()), display, SLOT(resetLabels()));
    }

    connect(this, SIGNAL(resetMachine()), tlbWindow, SIGNAL(onMachineReset()));

    connect(this, SIGNAL(stopSig()), clock, SLOT(stop()));
    connect(this, SIGNAL(stopSig()), toolbar, SLOT(stop()));

    connect(mac, SIGNAL(updateStatus(QString)), toolbar, SLOT(updateStatus(QString)));

    connect(this, SIGNAL(resetMachine()), debugger, SLOT(resetSymbolTable()));
    connect(debugger, SIGNAL(stabUpdated()), bpWindow, SLOT(updateContent()));
    connect(this, SIGNAL(resumeExec()), debugger, SIGNAL(MachineRan()));

    setCentralWidget(mainWidget);
}

void qarm::powerOn(){
    softReset();
    if(initialized)
        emit poweredOn();
}

void qarm::pause(){
    mac->refreshData(true);
    stop();
}

void qarm::softReset(){
    stop();
    initialized = false;
    emit resetMachine();
    toolbar->setSpeed(IPSMAX);
    doReset = false;
    if(!initialize()){
        emit poweredOff();
    }
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
            emit poweredOff();
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
        mac->setUIupdate(MC_Holder::getInstance()->getConfig()->getRefreshRate(), MC_Holder::getInstance()->getConfig()->getRefreshOnPause());
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
        QarmMessageBox *warning = new QarmMessageBox(QarmMessageBox::WARNING, "Warning",
                                                     "Machine not initialized,\ncannot display memory contents.", this);
        warning->show();
    }
}

void qarm::selectCore(){
    if(dataLoaded){
        QarmMessageBox *question = new QarmMessageBox(QarmMessageBox::QUESTION, "Caution",
                                                      "Program File already loaded..\nReset machine and load new Program?", this);
        question->show();

        if(question->result() == QarmMessageBox::Rejected)
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
        QarmMessageBox *question = new QarmMessageBox(QarmMessageBox::QUESTION, "Caution",
                                                      "BIOS ROM already loaded..\nReset machine and load new BIOS?", this);
        question->show();

        if(question->result() == QarmMessageBox::Rejected)
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
            QarmMessageBox *error = new QarmMessageBox(QarmMessageBox::CRITICAL, "Error", "Could not open Core file", this);
            error->show();
            return false;
        }
        QDataStream in(&f);
        ramMemory *ram = mac->getBus()->getRam();
        if(ram != NULL){
            Word len = (Word) f.size();
            char *buffer = new char[len];
            int sz = in.readRawData(buffer, len);
            if(sz <= 0 || (buffer[0] | buffer[1]<<8 | buffer[2]<<16 | buffer[3]<<24) != COREFILEID){
                QarmMessageBox *error = new QarmMessageBox(QarmMessageBox::CRITICAL, "Error", "Irregular Core file", this);
                error->show();
                return false;
            }
            if(sz <= 0 || !mac->getBus()->loadRAM(buffer, (Word) sz, true)){
                QarmMessageBox *error = new QarmMessageBox(QarmMessageBox::CRITICAL, "Error", "Problems while loading Core file", this);
                error->show();
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
            QarmMessageBox *error = new QarmMessageBox(QarmMessageBox::CRITICAL, "Error", "Could not open BIOS file", this);
            error->show();
            return false;
        }
        QDataStream in(&f);
        Word len = (Word) f.size();
        char *buffer = new char[len];
        Word sz = in.readRawData(buffer, len);
        if(sz <= 0 || (buffer[0] | buffer[1]<<8 | buffer[2]<<16 | buffer[3]<<24) != BIOSFILEID){
            QarmMessageBox *error = new QarmMessageBox(QarmMessageBox::CRITICAL, "Error", "Irregular BIOS file", this);
            error->show();
            return false;
        }
        sz -= 8;
        if(sz <= 0 || !mac->getBus()->loadBIOS(buffer+8, (Word) sz)){
            QarmMessageBox *error = new QarmMessageBox(QarmMessageBox::CRITICAL, "Error", "Problems while flashing BIOS ROM", this);
            error->show();
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
            QarmMessageBox *error = new QarmMessageBox(QarmMessageBox::CRITICAL, "Error", e.what(), this);
            error->show();
            return;
        }
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

void qarm::show(){
    QMainWindow::show();
    emit resetDisplay();
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

void qarm::closeFWindow(){
    application->focusWindow()->close();
}

void qarm::kill(){
    this->destroy();
}

#endif //QARM_QARM_CC
