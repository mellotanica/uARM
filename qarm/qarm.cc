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
#include "services/elf2arm.h"
#include "services/utility.h"

#include "qarm/qarmmessagebox.h"

#include <QFileDialog>
#include <QFile>
#include <QStandardPaths>
#include <QScrollArea>
#include <QMetaObject>
#include <QIcon>
#include <QWindow>

inline Word SWAP_ENDIANESS(Word wp){
    Byte *tp = (Byte *) &wp;
    Word ret = ((Byte) (*tp) | ((Byte) *(tp+1)) << 8 | ((Byte) *(tp+2)) << 16 | ((Byte) *(tp+3)) << 24);
    return ret;
}

qarm::qarm(QApplication *app, QFile *confFile, bool autorun, bool runandexit, bool gui):
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

    setWindowTitle("uARM");
    setWindowIcon(QIcon(LIB_PATH "icons/window_default-48.png"));

    mainWidget = new QWidget(this);
    toolbar = new mainBar(mainWidget);
    display = new procDisplay(mainWidget);

    mac = new machine(debugger->getBreakpoints(),debugger->getSuspects(),debugger->getTracepoints());

    toolbar->setSpeed(IPSMAX);

    centralLayout = new QVBoxLayout();

    centralLayout->addWidget(new QFLine(false));
    centralLayout->addWidget(toolbar);
    centralLayout->addWidget(new QFLine(false));
    centralLayout->addWidget(display);

    mainWidget->setLayout(centralLayout);

    clock = new QTimer(this);

    // FIXME: slow
    bpWindow = new breakpoint_window(mac, this);
    structWindow = new structures_window(mac, this);
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
    connect(this, SIGNAL(poweredOn()), debugger, SLOT(startMachine()));
    connect(this, SIGNAL(poweredOff()), toolbar, SLOT(doPowerOff()));

    connect(toolbar, SIGNAL(showBPW()), bpWindow, SLOT(show()));
    connect(toolbar, SIGNAL(hideBPW()), bpWindow, SLOT(hide()));
    connect(bpWindow, SIGNAL(hiding()), toolbar, SLOT(uncheckBPB()));
    connect(this, SIGNAL(resetMachine()), bpWindow, SLOT(reset()));

    connect(toolbar, SIGNAL(showTLB()), tlbWindow, SLOT(show()));
    connect(toolbar, SIGNAL(hideTLB()), tlbWindow, SLOT(hide()));
    connect(tlbWindow, SIGNAL(hiding()), toolbar, SLOT(uncheckTLB()));

    connect(toolbar, SIGNAL(showSTW()), structWindow, SLOT(show()));
    connect(toolbar, SIGNAL(hideSTW()), structWindow, SLOT(hide()));
    connect(structWindow, SIGNAL(hiding()), toolbar, SLOT(uncheckSTA()));
    connect(this, SIGNAL(resetMachine()), structWindow, SLOT(updateContent()));
    connect(mac, SIGNAL(dataReady(Word*,Word*,Word*,Word,Word,Word,QString)), structWindow, SLOT(update()));
    connect(structWindow, SIGNAL(openRamViewer(Word,Word,QString,bool)), this, SLOT(showRamSel(Word,Word,QString,bool)));

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
    connect(mac, SIGNAL(resetting()), this, SLOT(onMachineReset()));

    connect(this, SIGNAL(stopSig()), clock, SLOT(stop()));
    connect(this, SIGNAL(stopSig()), toolbar, SLOT(stop()));

    connect(mac, SIGNAL(updateStatus(QString)), toolbar, SLOT(updateStatus(QString)));

    connect(debugger, SIGNAL(stabUpdated()), bpWindow, SLOT(updateContent()));
    connect(debugger, SIGNAL(stabUpdated()), structWindow, SLOT(updateContent()));
    connect(this, SIGNAL(resumeExec()), debugger, SIGNAL(MachineRan()));

    setCentralWidget(mainWidget);

    if(autorun){
        if(runandexit){
            connect(mac, SIGNAL(executionTerminated(int)), this, SLOT(kill(int)));
        }
        toolbar->poweron();
        toolbar->run();
        if(gui){
            toolbar->showT0();
        }
    }
}

void qarm::powerOn(){
    softReset();
    if(initialized)
        emit poweredOn();
}

void qarm::powerOff(){
    stop();
    initialized = false;
    emit poweredOff();
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
    if(initialized){
        emit stopSig();
        resuming = true;
        mac->setUIupdate(0);
    }
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

void qarm::showRamSel(Word start, Word end, QString label, bool offset){
    if(initialized){
        ramView *ramWindow = new ramView(mac, start, end, label, offset, this);
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
        coreF = fileName.toStdString();
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
        biosF = fileName.toStdString();
        biosLoaded = true;
    }
}

bool qarm::openRAM(){
    const char *coreF = MC_Holder::getInstance()->getConfig()->getROM(ROM_TYPE_CORE).c_str();
    if(strcmp(coreF, "") != 0){
        coreElf *elf;
        if(MC_Holder::getInstance()->getConfig()->getExternalStab()){
            elf = new coreElf(coreF, MC_Holder::getInstance()->getConfig()->getSymbolTableASID(),
                               MC_Holder::getInstance()->getConfig()->getROM(ROM_TYPE_STAB).c_str());
        } else {
            elf = new coreElf(coreF, MC_Holder::getInstance()->getConfig()->getSymbolTableASID());
        }
        if(!elf->allRight()){
            QarmMessageBox *error = new QarmMessageBox(QarmMessageBox::CRITICAL, "BIOS Error", elf->getError(), this);
            error->show();
            return false;
        }
        Word address = SWAP_ENDIANESS(((Word)elf->header[AOUT_HE_TEXT_VADDR]));
        Word dataVAddr = SWAP_ENDIANESS(((Word)elf->header[AOUT_HE_DATA_VADDR]));
        Word dataOffset = SWAP_ENDIANESS(((Word)elf->header[AOUT_HE_DATA_OFFSET]));
        Word totsize = elf->header[AOUT_HE_TEXT_MEMSZ] + elf->header[AOUT_HE_DATA_MEMSZ];
        bool text = true;

        //copy provided data only for legit ram addresses
        for(Word i = 0; i < totsize; i++, address++){
            Byte mb = (text ? elf->readTextByte() : elf->readDataByte());
            if(address >= RAMBASEADDR){
                if(i >= dataOffset && text){
                    address = dataVAddr;
                    text = false;
                    mb = elf->readDataByte();
                }
                mac->getBus()->writeB(&address, mb);
            }
        }

        //now make sure core header is at its place at the beginning of ram
        elf->seekText();
        for(Word i = 0, address = RAMBASEADDR; i < WS*N_AOUT_HDR_ENT; i++, address++){
            mac->getBus()->writeB(&address, elf->readTextByte());
        }

        //setup symbol table
        DebuggerHolder::getInstance()->getDebugSession()->setSymbolTable(elf->getSymbolTable());
        delete elf;
    }
    return true;
}

bool qarm::openBIOS(){
    const char *bfile = MC_Holder::getInstance()->getConfig()->getROM(ROM_TYPE_BIOS).c_str();
    if(strcmp(bfile, "") != 0){
        biosElf *elf = new biosElf(bfile);
        Word address, size;
        if(!elf->allRight()){
            QarmMessageBox *error = new QarmMessageBox(QarmMessageBox::CRITICAL, "BIOS Error", elf->getError(), this);
            error->show();
            return false;
        }
        size = elf->getSize();
        mac->getBus()->initBIOS(size);
        address = BIOSBASEADDR;
        for(Word i = 0; i < size; i++, address++){
            mac->getBus()->writeB(&address, elf->readByte());
        }

        delete elf;
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

void qarm::disableMainbar(bool fromConfig, bool value){
    if(fromConfig){
        if(toolbar->getDisabled()){
            powerOff();
            emit resetMachine();
        }
    } else {
        toolbar->disableMainbar(value);
    }
}

void qarm::onMachineReset(){
    disableMainbar(false, false);
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

void qarm::kill(int returncode){
    application->closeAllWindows();
    application->exit(returncode);
}

#endif //QARM_QARM_CC
