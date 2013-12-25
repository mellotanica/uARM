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

mainBar::mainBar(QWidget *parent) :
    QToolBar(parent)
{
    setMovable(false);
    setFloatable(false);
    setOrientation(Qt::Horizontal);

    pauseIco = new QIcon(LIB_PREF PAUSEICON);
    playIco = new QIcon(LIB_PREF PLAYICON);

    configB = new styledButton();
    configB->setToolTip("Machine Configs");
    configB->setIcon(QIcon(LIB_PREF CONFIGICON));

    playB = new styledButton();
    playB->setToolTip("Play");
    playB->setCheckable(true);
    playB->setIcon(*playIco);
    playB->setEnabled(false);

    resetB = new styledButton();
    resetB->setToolTip("Reset");
    resetB->setIcon(QIcon(LIB_PREF POWERONICON));

    stepB = new styledButton();
    stepB->setToolTip("Step");
    stepB->setIcon(QIcon(LIB_PREF STEPICON));
    stepB->setEnabled(false);

    plusMinusW = new QWidget;
    plusMinusL = new QVBoxLayout;

    plusB = new styledButton();
    plusB->setText("+");

    minusB = new styledButton();
    minusB->setText("-");

    plusMinusL->addWidget(plusB);
    plusMinusL->addWidget(minusB);

    plusMinusW->setLayout(plusMinusL);

    scrollerL = new QVBoxLayout;
    QHBoxLayout *topLay = new QHBoxLayout;
    scrollerW = new QWidget;

    speedLab = new QLabel("50 instr/sec");
    speedLab->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    speedLab->setAlignment(Qt::AlignRight);
    speedLab->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    statusLab = new QLabel("HALTED");
    statusLab->setFrameStyle(QFrame::StyledPanel);
    statusLab->setAlignment(Qt::AlignRight);
    statusLab->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    speedSl = new QSlider();
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

    utilsW = new QWidget;
    utilsL = new QVBoxLayout;

    ramB = new styledButton();
    ramB->setToolButtonStyle(Qt::ToolButtonTextOnly);
    ramB->setText("View Ram");

    for (unsigned int i = 0; i < N_DEV_PER_IL; ++i) {
        showTerminalActions[i] = new QAction(QString("Terminal %1").arg(i), this);
        showTerminalActions[i]->setShortcut(QKeySequence(QString("Alt+%1").arg(i)));
        showTerminalActions[i]->setData(QVariant(i));
        connect(showTerminalActions[i], SIGNAL(triggered()), this, SLOT(showTerminalClicked()));
        showTerminalActions[i]->setEnabled(false);
    }

    windowB = new styledButton();
    windowB->setText("Terminals");

    windowMenu = new QMenu;

    for (unsigned int i = 0; i < N_DEV_PER_IL; ++i)
        windowMenu->addAction(showTerminalActions[i]);

    /*windowDropDown = new QMenuBar(windowB);
    windowDropDown->addMenu(windowMenu);
    windowDropDown->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    windowDropDown->setVisible(false);*/

    utilsL->addWidget(ramB);
    utilsL->addWidget(windowB);
    utilsW->setLayout(utilsL);

    addWidget(configB);
    addWidget(playB);
    addWidget(resetB);
    addWidget(stepB);
    addWidget(plusMinusW);
    addWidget(scrollerW);
    addWidget(utilsW);

    connect(this, SIGNAL(speedChanged(int)), this, SLOT(setSpeedLab(int)));
    connect(speedSl, SIGNAL(valueChanged(int)), this, SIGNAL(speedChanged(int)));
    connect(playB, SIGNAL(toggled(bool)), this, SLOT(playToggled(bool)));
    connect(resetB, SIGNAL(clicked()), this, SLOT(poweron()));
    connect(stepB, SIGNAL(clicked()), this, SIGNAL(step()));
    connect(stepB, SIGNAL(clicked()), this, SLOT(stop()));
    connect(ramB, SIGNAL(clicked()), this, SIGNAL(showRam()));
    connect(plusB, SIGNAL(clicked()), this, SLOT(plus()));
    connect(minusB, SIGNAL(clicked()), this, SLOT(minus()));
    connect(configB, SIGNAL(clicked()), this, SIGNAL(showConfig()));
    connect(windowB, SIGNAL(clicked()), this, SLOT(showDropDownMenu()));
}

void mainBar::poweron(){
    QIcon *resetIco = new QIcon(LIB_PREF RESETICON);
    disconnect(resetB, SIGNAL(clicked()), this, SLOT(poweron()));
    connect(resetB, SIGNAL(clicked()), this, SLOT(resetPressed()));
    playB->setEnabled(true);
    stepB->setEnabled(true);
    resetB->setToolTip("Reset Machine");
    resetB->setIcon(*resetIco);
    resetPressed();
}

void mainBar::playToggled(bool checked){
    if(checked){
        playB->setIcon(*pauseIco);
        playB->setToolTip("Pause");
        int spV = speedSl->value();
        emit play(spV);
    } else {
        playB->setIcon(*playIco);
        playB->setToolTip("Play");
        emit pause();
    }
}

void mainBar::resetPressed(){
    if(playB->isChecked())
        playB->toggle();
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

void mainBar::showTerminalClicked(){
    QAction* action = static_cast<QAction*>(sender());
    unsigned int devNo = action->data().toUInt();
    emit showTerminal(devNo);
}

#endif //QARM_MAINBAR_CC
