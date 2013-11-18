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

#include "qarm/mainbar.h"
#include "qarm/guiConst.h"
#include "armProc/const.h"

mainBar::mainBar(QWidget *parent) :
    QToolBar(parent)
{
    setMovable(false);
    setFloatable(false);
    setOrientation(Qt::Horizontal);

    playIco = new QIcon(PLAYICON);
    resetIco = new QIcon(RESETICON);
    pauseIco = new QIcon(PAUSEICON);
    stepIco = new QIcon(STEPICON);

    openMenu = new QMenu("Open");
    openMenu->addAction("BIOS ROM", this, SIGNAL(openBIOS()));
    openMenu->addAction("Program Image", this, SIGNAL(openRAM()));

    playB = new styledButton();
    playB->setToolTip("Play");
    playB->setCheckable(true);
    playB->setIcon(*playIco);

    resetB = new styledButton();
    resetB->setToolTip("Reset");
    resetB->setIcon(*resetIco);

    stepB = new styledButton();
    stepB->setToolTip("Step");
    stepB->setIcon(*stepIco);

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

    ramB = new styledButton();
    ramB->setToolButtonStyle(Qt::ToolButtonTextOnly);
    ramB->setText("View Ram");

    openDropDown = new QMenuBar();
    openDropDown->addMenu(openMenu);
    openDropDown->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

    addWidget(openDropDown);
    addWidget(playB);
    addWidget(resetB);
    addWidget(stepB);
    addWidget(plusMinusW);
    addWidget(scrollerW);
    addWidget(ramB);

    connect(this, SIGNAL(speedChanged(int)), this, SLOT(setSpeedLab(int)));
    connect(speedSl, SIGNAL(valueChanged(int)), this, SIGNAL(speedChanged(int)));
    connect(playB, SIGNAL(toggled(bool)), this, SLOT(playToggled(bool)));
    connect(resetB, SIGNAL(clicked()), this, SLOT(resetPressed()));
    connect(stepB, SIGNAL(clicked()), this, SIGNAL(step()));
    connect(stepB, SIGNAL(clicked()), this, SLOT(stop()));
    connect(ramB, SIGNAL(clicked()), this, SIGNAL(showRam()));
    connect(plusB, SIGNAL(clicked()), this, SLOT(plus()));
    connect(minusB, SIGNAL(clicked()), this, SLOT(minus()));
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

void mainBar::setSpeedLab(int spV){
    QString text;
    if(spV <= IPSTRESH){
        if(spV < 10)
            text = "  " + QString::number(spV) + " instr/sec";
        else
            text = QString::number(spV) + " instr/sec";
    } else
            text = "        MAX Mz";
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
