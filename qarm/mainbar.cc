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

#ifndef QARM_MAINBAR_CC
#define QARM_MAINBAR_CC

#include "qarm/mainbar.h"
#include "qarm/guiConst.h"
#include "qarm/qarm.h"
//#include "armProc/const.h"

#include "services/debug.h"

mainBar::mainBar(QWidget *parent) :
    QToolBar(parent)
{
    onState = false;

    setMovable(false);
    setFloatable(false);
    setOrientation(Qt::Horizontal);

    pauseIco = new QIcon(LIB_PATH "icons/pause.png");
    playIco = new QIcon(LIB_PATH "icons/play.png");

    configB = new styledButton(this);
    configB->setAccessibleName("Machine Configs");
    configB->setToolTip(configB->accessibleName());
    configB->setIcon(QIcon(LIB_PATH "icons/config.png"));

    playB = new styledButton(this);
    playB->setAccessibleName("Play");
    playB->setToolTip(playB->accessibleName());
    playB->setCheckable(true);
    playB->setIcon(*playIco);
    playB->setEnabled(false);

    resetB = new styledButton(this);
    resetB->setAccessibleName("Power on");
    resetB->setToolTip(resetB->accessibleName());
    resetB->setIcon(QIcon(LIB_PATH "icons/poweron.png"));

    stepB = new styledButton(this);
    stepB->setAccessibleName("Step");
    stepB->setToolTip(stepB->accessibleName());
    stepB->setIcon(QIcon(LIB_PATH "icons/step.png"));
    stepB->setEnabled(false);

    plusMinusW = new QWidget(this);
    plusMinusL = new QVBoxLayout();

    plusB = new styledButton(plusMinusW);
    plusB->setText("+");
    plusB->setAccessibleName("Increase Emulation Speed");

    minusB = new styledButton(plusMinusW);
    minusB->setText("-");
    minusB->setAccessibleName("Decrease Emulation Speed");

    plusMinusL->addWidget(plusB);
    plusMinusL->addWidget(minusB);

    plusMinusW->setLayout(plusMinusL);

    scrollerW = new QWidget(this);
    scrollerL = new QVBoxLayout();
    QHBoxLayout *topLay = new QHBoxLayout();

    speedLab = new QLabel("50 instr/sec", scrollerW);
    speedLab->setAccessibleName("Emulation Speed");
    speedLab->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    speedLab->setAlignment(Qt::AlignRight);
    speedLab->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    statusLab = new QLabel("HALTED", scrollerW);
    statusLab->setAccessibleName("System Status");
    statusLab->setFrameStyle(QFrame::StyledPanel);
    statusLab->setAlignment(Qt::AlignRight);
    statusLab->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    speedSl = new QSlider(scrollerW);
    speedSl->setAccessibleName("Emulation Speed Selection");
    speedSl->setOrientation(Qt::Horizontal);
    speedSl->setMinimum(IPSMIN);
    speedSl->setMaximum(IPSMAX);

    scrollerL->addLayout(topLay);
    scrollerL->addWidget(speedSl);

    topLay->addWidget(speedLab);
    topLay->addSpacerItem(new QSpacerItem(1,1,QSizePolicy::Expanding, QSizePolicy::Minimum));
    topLay->addWidget(statusLab);

    scrollerW->setLayout(scrollerL);

    setSpeedLab(speedSl->value());

    utilsW = new QWidget(this);
    utilsL = new QVBoxLayout();
    utilsUpperL = new QHBoxLayout();
    utilsLowerL = new QHBoxLayout();

    ramB = new styledButton(utilsW);
    ramB->setText("Memory");
    ramB->setAccessibleName("Memory contents viewers Menu");
    ramMenu = new QMenu(ramB);

    ramAction = new QAction(QString("Bus Viewer"), ramMenu);
    ramAction->setEnabled(true);
    connect(ramAction, SIGNAL(triggered()), this, SIGNAL(showRam()));
    ramMenu->addAction(ramAction);

    structsAction = new QAction(QString("Structures Viewer"), ramMenu);
    structsAction->setEnabled(true);
    structsAction->setCheckable(true);
    connect(structsAction, SIGNAL(toggled(bool)), this, SLOT(toggleStructsViewer(bool)));
    ramMenu->addAction(structsAction);

    windowB = new styledButton(utilsW);
    windowB->setText("Terminals");
    windowB->setAccessibleName("Show Terminals Menu");
    windowB->setEnabled(false);
    windowMenu = new QMenu(windowB);

    for (unsigned int i = 0; i < N_DEV_PER_IL; ++i) {
        showTerminalActions[i] = new QAction(QString("Show Terminal %1").arg(i), windowMenu);
        showTerminalActions[i]->setShortcut(QKeySequence(QString("Alt+%1").arg(i)));
        showTerminalActions[i]->setData(QVariant(i));
        connect(showTerminalActions[i], SIGNAL(triggered()), this, SLOT(showTerminalClicked()));
        showTerminalActions[i]->setEnabled(false);
    }

    for (unsigned int i = 0; i < N_DEV_PER_IL; ++i)
        windowMenu->addAction(showTerminalActions[i]);

    breakpB = new styledButton(utilsW);
    breakpB->setText("Breakpoints");
    breakpB->setAccessibleName("Breakpoint Selector");
    breakpB->setCheckable(true);
    breakpB->setEnabled(true);

    tlbB = new styledButton(utilsW);
    tlbB->setText("TLB");
    tlbB->setAccessibleName("TLB contents viewer");
    tlbB->setCheckable(true);
    tlbB->setEnabled(true);

    utilsUpperL->addWidget(breakpB);
    utilsUpperL->addWidget(new QWidget());
    utilsUpperL->addWidget(tlbB);
    utilsLowerL->addWidget(windowB);
    utilsLowerL->addWidget(new QWidget());
    utilsLowerL->addWidget(ramB);

    utilsL->addLayout(utilsUpperL);
    utilsL->addLayout(utilsLowerL);
    utilsW->setLayout(utilsL);

    connect(this, SIGNAL(speedChanged(int)), this, SLOT(setSpeedLab(int)));
    connect(speedSl, SIGNAL(valueChanged(int)), this, SIGNAL(speedChanged(int)));
    connect(playB, SIGNAL(toggled(bool)), this, SLOT(playToggled(bool)));
    connect(resetB, SIGNAL(clicked()), this, SIGNAL(powerOn()));
    connect(stepB, SIGNAL(clicked()), this, SIGNAL(step()));
    connect(stepB, SIGNAL(clicked()), this, SLOT(stop()));
    connect(plusB, SIGNAL(clicked()), this, SLOT(plus()));
    connect(minusB, SIGNAL(clicked()), this, SLOT(minus()));
    connect(configB, SIGNAL(clicked()), this, SIGNAL(showConfig()));
    connect(windowB, SIGNAL(clicked()), this, SLOT(showDropDownMenu()));
    connect(ramB, SIGNAL(clicked()), this, SLOT(showRamMenu()));
    connect(breakpB, SIGNAL(toggled(bool)), this, SLOT(toggleBPButton(bool)));
    connect(tlbB, SIGNAL(toggled(bool)), this, SLOT(toggleTlbViewer(bool)));

    debugSignaler *debugger = debugSignaler::getInstance();
    connect(debugger, SIGNAL(pause()), this, SLOT(stop()));

    this->addWidget(configB);
    this->addWidget(playB);
    this->addWidget(resetB);
    this->addWidget(stepB);
    this->addWidget(plusMinusW);
    this->addWidget(scrollerW);
    this->addWidget(utilsW);
}

void mainBar::poweron(){
    resetB->click();
}

void mainBar::run(){
    while(!playB->isEnabled())
        ;
    playB->click();
}

void mainBar::showT0(){
    showTerminal(0);
}

void mainBar::doPowerOn(){
    QIcon *resetIco = new QIcon(LIB_PATH "icons/reset.png");
    disconnect(resetB, SIGNAL(clicked()), this, SIGNAL(powerOn()));
    connect(resetB, SIGNAL(clicked()), this, SLOT(resetPressed()));
    playB->setEnabled(true);
    stepB->setEnabled(true);
    windowB->setEnabled(true);
    resetB->setAccessibleName("Reset Machine");
    resetB->setToolTip(resetB->accessibleName());
    resetB->setIcon(*resetIco);
    onState = true;
}

void mainBar::doPowerOff(){
    QIcon *resetIco = new QIcon(LIB_PATH "icons/poweron.png");
    disconnect(resetB, SIGNAL(clicked()), this, SLOT(resetPressed()));
    connect(resetB, SIGNAL(clicked()), this, SIGNAL(powerOn()));
    playB->setEnabled(false);
    stepB->setEnabled(false);
    windowB->setEnabled(false);
    resetB->setAccessibleName("Power on");
    resetB->setToolTip(resetB->accessibleName());
    resetB->setIcon(*resetIco);
    onState = false;
}

void mainBar::playToggled(bool checked){
    if(checked){
        playB->setIcon(*pauseIco);
        playB->setAccessibleName("Pause");
        playB->setToolTip(playB->accessibleName());
        int spV = speedSl->value();
        emit play(spV);
    } else {
        playB->setIcon(*playIco);
        playB->setAccessibleName("Play");
        playB->setToolTip(playB->accessibleName());
        emit pause();
    }
}

void mainBar::resetPressed(){
    stop();
    emit reset();
}

void mainBar::setSpeed(int speedVal){
    speedSl->setValue(speedVal);
}

void mainBar::setSpeedLab(int spV){
    QString text;
    if(spV <= IPSTRESH){
        if(spV < 10)
            text = "  " + QString::number(spV) + " instr/sec";
        else
            text = QString::number(spV) + " instr/sec";
    } else
            text = "        MAX MHz";
    speedLab->setText(text);
}

void mainBar::stop(){
    if(playB->isChecked())
        playB->toggle();
}

void mainBar::plus(){
    int val = speedSl->value();
    if(val < IPSTRESH)
        speedSl->setValue(val+IPSSTEP);
    else
        speedSl->setValue(speedSl->maximum());
}

void mainBar::minus(){
    int val = speedSl->value();
    if(val > IPSTRESH)
        speedSl->setValue(IPSTRESH);
    else
        speedSl->setValue(val-IPSSTEP);
}

void mainBar::updateStatus(QString state){
    statusLab->setText(state);
}

void mainBar::setTerminalEnabled(unsigned int devNo, bool enabled){
    showTerminalActions[devNo]->setEnabled(enabled);
}

void mainBar::showDropDownMenu(){
    windowMenu->popup(windowB->mapToGlobal(QPoint(0,windowB->height())));
}

void mainBar::showRamMenu(){
    ramMenu->popup(ramB->mapToGlobal(QPoint(0,ramB->height())));
}

void mainBar::showTerminalClicked(){
    QAction* action = static_cast<QAction*>(sender());
    unsigned int devNo = action->data().toUInt();
    emit showTerminal(devNo);
}

void mainBar::toggleBPButton(bool checked){
    if(checked)
        emit showBPW();
    else
        emit hideBPW();
}

void mainBar::uncheckBPB(){
    breakpB->setChecked(false);
}

void mainBar::toggleTlbViewer(bool status){
    if(status)
        emit showTLB();
    else
        emit hideTLB();
}

void mainBar::uncheckTLB(){
    tlbB->setChecked(false);
}

void mainBar::toggleStructsViewer(bool status){
    if(status)
        emit showSTW();
    else
        emit hideSTW();
}

void mainBar::uncheckSTA(){
    structsAction->setChecked(false);
}

#endif //QARM_MAINBAR_CC
