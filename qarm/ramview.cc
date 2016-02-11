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

#ifndef QARM_RAMVIEW_CC
#define QARM_RAMVIEW_CC

#include "ramview.h"
#include "qarm/qarmmessagebox.h"
#include <QHBoxLayout>
#include <QLabel>
#include "armProc/machine_config.h"

ramView::ramView(machine *mac, QWidget *parent) :
    mac(mac),
    QWidget(parent)
{
    setWindowFlags(Qt::Window);
    setWindowTitle("Ram Inspector");
    mainLayout = new QVBoxLayout();
    QHBoxLayout *topPanel = new QHBoxLayout();
    mainLayout->addLayout(topPanel);

    QRegExp rx("[0-9,a-f]{1,8}", Qt::CaseInsensitive);
    QRegExpValidator *hexValidator = new QRegExpValidator(rx, this);

    startEd = new QLineEdit(this);
    startEd->setAccessibleName("Start Address");
    startEd->setToolTip(startEd->accessibleName());
    startEd->setAccessibleDescription("Start address in hexadecimal format without leading 0x");
    startEd->setValidator(hexValidator);
    startEd->setMaxLength(8);

    endEd = new QLineEdit(this);
    endEd->setAccessibleName("End Address");
    endEd->setToolTip(endEd->accessibleName());
    endEd->setAccessibleDescription("End address in hexadecimal format without leading 0x");
    endEd->setValidator(hexValidator);
    endEd->setMaxLength(8);

    visualizeB = new QPushButton("Display Portion", this);
    visualizeB->setAccessibleName("Display Selected Portion");

    topPanel->addWidget(new QLabel("0x", this));
    topPanel->addWidget(startEd);
    topPanel->addWidget(new QLabel("-> 0x", this));
    topPanel->addWidget(endEd);
    topPanel->addWidget(visualizeB);

    setLayout(mainLayout);
    hide();

    connect(visualizeB, SIGNAL(clicked()), this, SLOT(visualize()));
}

void ramView::newRamLabel(QWidget *parent){
    ramLabel = new QLineEdit(parent);
    ramLabel->setAccessibleName("Ram Portion Label");
    ramLabel->setToolTip(ramLabel->accessibleName());
    ramLabel->setAccessibleDescription("Editable label usefull to note down ram portion meaning");
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

    if(start > end || (end - start) > MAX_VIEWABLE_RAM){
        mainLayout->removeWidget(ramLabel);
        mainLayout->removeWidget(ramViewer);
        delete ramLabel;
        delete ramViewer;
        char *message;
        if(start > end){
            message = "Start address cannot be higher than End address...";
        } else {
            message = "Memory segment too large,\n max displayble size: 10 KB";
        }
        QarmMessageBox *warning = new QarmMessageBox(QarmMessageBox::WARNING, "Warning", message, this);
        warning->show();
    } else if(conv && (start != startAddr || end != endAddr)){

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
            ramLabel->clear();
        } else {
            newRamLabel(this);
            mainLayout->addWidget(ramLabel);
        }
        ramViewer = new HexView(start, end, mac, this);
        mainLayout->addWidget(ramViewer);
    }
}

#endif //QARM_RAMVIEW_CC
