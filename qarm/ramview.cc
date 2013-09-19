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

#include "ramview.h"
#include <QHBoxLayout>
#include <QLabel>

ramView::ramView(machine *mac, QWidget *parent) :
    mac(mac),
    QWidget(parent)
{
    setWindowFlags(Qt::Window);
    mainLayout = new QVBoxLayout;
    QHBoxLayout *topPanel = new QHBoxLayout;
    mainLayout->addLayout(topPanel);

    startEd = new QLineEdit;
    startEd->setToolTip("Start Address");
    startEd->setInputMask("HHHHHHHH");
    startEd->setMaxLength(8);

    endEd = new QLineEdit;
    endEd->setToolTip("End Address");
    endEd->setInputMask("HHHHHHHH");
    startEd->setMaxLength(8);

    visualizeB = new QPushButton("Display Portion");

    topPanel->addWidget(new QLabel("0x"));
    topPanel->addWidget(startEd);
    topPanel->addWidget(new QLabel("0x"));
    topPanel->addWidget(endEd);
    topPanel->addWidget(visualizeB);

    setLayout(mainLayout);
    hide();

    connect(visualizeB, SIGNAL(clicked()), this, SLOT(visualize()));
}

void ramView::update(){
    if(ramViewer != NULL)
        ramViewer->Refresh();
}

void ramView::visualize(){
    bool conv = true, res;
    Word start = startEd->text().toUInt(&res,16);
    conv &= res;
    Word end = endEd->text().toUInt(&res,16);
    conv &= res;

    if(conv && (start != startAddr || end != endAddr)){

        if(start & 3){
            start &= 0xFFFFFFFC;
            startEd->setText(QString::number(start, 16));
        }
        if(end & 3){
            end &= 0xFFFFFFFC;
            endEd->setText(QString::number(end, 16));
        }
        startAddr = start;
        endAddr = end;

        if(ramViewer != NULL){
            mainLayout->removeWidget(ramViewer);
            delete ramViewer;
        }
        ramViewer = new HexView(start, end, mac);
        mainLayout->addWidget(ramViewer);
    }
}
