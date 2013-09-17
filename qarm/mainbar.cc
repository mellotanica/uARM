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

#include "mainbar.h"
#include "guiConst.h"
#include <QFileDialog>

mainBar::mainBar(QWidget *parent) :
    QToolBar(parent)
{
    setMovable(false);
    setFloatable(false);
    setOrientation(Qt::Horizontal);

    playIco = new QIcon(PLAYICON);
    stopIco = new QIcon(STOPICON);
    pauseIco = new QIcon(PAUSEICON);
    stepIco = new QIcon(STEPICON);

    openB = new styledButton();
    openB->setToolButtonStyle(Qt::ToolButtonTextOnly);
    openB->setText("Open");

    playB = new styledButton();
    playB->setCheckable(true);
    playB->setIcon(*playIco);

    stopB = new styledButton();
    stopB->setIcon(*stopIco);

    stepB = new styledButton();
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
    scrollerW = new QWidget;

    speedLab = new QLabel("50 instr/sec");
    speedLab->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    speedLab->setAlignment(Qt::AlignRight);
    speedLab->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    speedSl = new QSlider();
    speedSl->setOrientation(Qt::Horizontal);
    speedSl->setMinimum(IPSMIN);
    speedSl->setMaximum(IPSMAX);

    scrollerL->addWidget(speedLab);
    scrollerL->addWidget(speedSl);

    scrollerW->setLayout(scrollerL);

    setSpeedLab(speedSl->value());

    ramB = new styledButton();
    ramB->setToolButtonStyle(Qt::ToolButtonTextOnly);
    ramB->setText("View Ram");

    addWidget(openB);
    addWidget(playB);
    addWidget(stopB);
    addWidget(stepB);
    addWidget(plusMinusW);
    addWidget(scrollerW);
    addWidget(ramB);

    connect(this, SIGNAL(speedChanged(int)), this, SLOT(setSpeedLab(int)));
    connect(speedSl, SIGNAL(valueChanged(int)), this, SIGNAL(speedChanged(int)));
    connect(playB, SIGNAL(toggled(bool)), this, SLOT(playToggled(bool)));
    connect(stopB, SIGNAL(clicked()), this, SLOT(stopPressed()));
    connect(stepB, SIGNAL(clicked()), this, SIGNAL(step()));
    connect(openB, SIGNAL(clicked()), this, SLOT(openPressed()));
    connect(ramB, SIGNAL(clicked()), this, SIGNAL(showRam()));
    connect(plusB, SIGNAL(clicked()), this, SLOT(plus()));
    connect(minusB, SIGNAL(clicked()), this, SLOT(minus()));
}

void mainBar::playToggled(bool checked){
    if(checked){
        playB->setIcon(*pauseIco);
        int spV = speedSl->value();
        emit play(spV);
    } else {
        playB->setIcon(*playIco);
        emit pause();
    }
}

void mainBar::stopPressed(){
    if(playB->isChecked())
        playB->toggle();
    emit stop();
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

void mainBar::openPressed(){
    QString fileName = QFileDialog::getOpenFileName(this, "Open Program File", "", "Binary Files (*.bin);;All Files (*.*)");
    if(fileName != "")
        emit open(fileName);
}
