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

    speedLab = new QLabel();

    speedSl = new QSlider();
    speedSl->setOrientation(Qt::Horizontal);

    setSpeedLab(speedSl->value());

    ramB = new styledButton();
    ramB->setToolButtonStyle(Qt::ToolButtonTextOnly);
    ramB->setText("View Ram");

    addWidget(openB);
    addWidget(playB);
    addWidget(stopB);
    addWidget(stepB);
    addWidget(plusMinusW);
    addWidget(speedLab);
    addWidget(speedSl);
    addWidget(ramB);

    connect(this, SIGNAL(speedChanged(int)), this, SLOT(setSpeedLab(int)));
    connect(speedSl, SIGNAL(valueChanged(int)), this, SIGNAL(speedChanged(int)));
    connect(playB, SIGNAL(toggled(bool)), this, SLOT(playToggled(bool)));
    connect(stopB, SIGNAL(clicked()), this, SIGNAL(stop()));
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

void mainBar::setSpeedLab(int spV){
    //stabilire a quanto corrisponde spV e impostare text
    QString text = QString::number(spV);
    speedLab->setText(text);
}

void mainBar::plus(){
    speedSl->setValue(speedSl->value()+sliderStep);
}

void mainBar::minus(){
    speedSl->setValue(speedSl->value()-sliderStep);
}

void mainBar::openPressed(){
    QString fileName = QFileDialog::getOpenFileName(this, "Open Program File", "", "Binary Files (*.bin);;All Files (*.*)");
    if(fileName != "")
        emit open(fileName);
}
