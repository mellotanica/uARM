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
#include <QSizePolicy>
#include "armProc/machine_config.h"

ramView::ramView(machine *mac, QWidget *parent) :
    QWidget(parent),
    offset(false)
{
    initRamView(mac);
}

ramView::ramView(machine *mac, Word start, Word end, QString label, bool offset, QWidget *parent):
    QWidget(parent),
    offset(offset)
{
    initRamView(mac);

    refreshBounds(start, end);
    labelText = label;

    visualize();
}

void ramView::initRamView(machine *mac){
    this->mac = mac;

    setWindowFlags(Qt::Window);
    setWindowTitle("Bus Inspector");
    mainLayout = new QVBoxLayout();
    QHBoxLayout *topPanel = new QHBoxLayout();
    mainLayout->addLayout(topPanel);
    topPanel->setSizeConstraint(QLayout::SetMinAndMaxSize);

    QRegExp rx("[0-9,a-f]{1,8}", Qt::CaseInsensitive);
    QRegExpValidator *hexValidator = new QRegExpValidator(rx, this);

    labelText = QString("");

    startEd = new QLineEdit(this);
    startEd->setAccessibleName("Start Address");
    startEd->setToolTip(startEd->accessibleName());
    startEd->setAccessibleDescription("Start address in hexadecimal format without leading 0x");
    startEd->setValidator(hexValidator);
    startEd->setMaxLength(8);

    endEd = new QLineEdit(this);
    setSecondField(offset);
    endEd->setValidator(hexValidator);
    endEd->setMaxLength(8);

    visualizeB = new QPushButton("Display Portion", this);
    visualizeB->setAccessibleName("Display Selected Portion");

    topPanel->addWidget(new QLabel("0x", this));
    topPanel->addWidget(startEd);
    topPanel->addWidget(new QLabel("->", this));

    offsetButton = new QToolButton(this);
    offsetButton->setText("+");
    offsetButton->setAccessibleName("Second field is size");
    offsetButton->setToolTip(offsetButton->accessibleName());
    offsetButton->setCheckable(true);
    offsetButton->setChecked(offset);
    offsetButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(offsetButton, SIGNAL(clicked(bool)), this, SLOT(toggleOffset(bool)));
    topPanel->addWidget(offsetButton);

    topPanel->addWidget(new QLabel("0x", this));
    topPanel->addWidget(endEd);
    topPanel->addWidget(visualizeB);

    setLayout(mainLayout);
    hide();

    connect(visualizeB, SIGNAL(clicked()), this, SLOT(visualize()));
}

void ramView::setSecondField(bool val){
    if(val){
        endEd->setAccessibleName("Region Size");
        endEd->setToolTip(endEd->accessibleName());
        endEd->setAccessibleDescription("Bus region size in hexadecimal format without leading 0x");
    } else {
        endEd->setAccessibleName("End Address");
        endEd->setToolTip(endEd->accessibleName());
        endEd->setAccessibleDescription("End address in hexadecimal format without leading 0x");
    }
}

void ramView::toggleOffset(bool val){
    offset = val;
    setSecondField(val);
}

void ramView::newRamLabel(QWidget *parent){
    ramLabel = new QLineEdit(parent);
    ramLabel->setAccessibleName("Portion Label");
    ramLabel->setToolTip(ramLabel->accessibleName());
    ramLabel->setAccessibleDescription("Editable label usefull to note down ram portion meaning");
    ramLabel->setText(labelText);
}

void ramView::refreshBounds(Word start, Word end){
    setLablelVal(startEd,start);
    if(end != start){
        setLablelVal(endEd, end);
    }
}

void ramView::update(){
    QString symbol = QString("");
    if(ramViewer != NULL){
        if(ramLabel != NULL){
            symbol = ramLabel->text();
        }
        if(symbol.isEmpty()){
            ramViewer->Refresh();
        } else {
            ramViewer->Refresh(symbol);
            refreshBounds(ramViewer->getStart(), ramViewer->getEnd());
        }
    }
}

void ramView::visualize(){
    if(startEd->text().isEmpty()){
        QarmMessageBox *warning = new QarmMessageBox(QarmMessageBox::WARNING, "Warning", "You must enter a valid adress in Start field", this);
        warning->show();
    } else {
        bool conv = true, res;
        Word start = startEd->text().toUInt(&res,16);
        conv &= res;
        Word end, off = 0;
        if(endEd->text().isEmpty())
            end = start;
        else {
            end = endEd->text().toUInt(&res,16);
            if(offset){
                off = end;
                end += start;
            }
        }
        conv &= res;

        if(start > end || (end - start) > MAX_VIEWABLE_RAM){
            // manually setting fields to NULL is required since delete alone seems to be not enough and can lead to double free
            if(ramLabel != NULL){
                labelText = ramLabel->text();
                mainLayout->removeWidget(ramLabel);
                delete ramLabel;
                ramLabel = NULL;
            }
            if(ramViewer != NULL){
                mainLayout->removeWidget(ramViewer);
                delete ramViewer;
                ramViewer = NULL;
            }
            char *message;
            if(start > end){
                message = (char *)"Start address cannot be higher than End address...";
            } else {
                message = (char *)"Memory segment too large,\n max displayble size: 10 KB";
            }
            QarmMessageBox *warning = new QarmMessageBox(QarmMessageBox::WARNING, "Warning", message, this);
            warning->show();
        } else if(conv && (start != startAddr || end != endAddr)){

            if(start & 3){
                start &= 0xFFFFFFFC;
                setLablelVal(startEd, start);
            }
            if(end & 3){
                if(offset){
                    off &= 0xFFFFFFFC;
                    setLablelVal(endEd,off);
                } else {
                    end &= 0xFFFFFFFC;
                    setLablelVal(endEd, end);
                }
            }
            startAddr = start;
            if(!offset)
                endAddr = end;

            if(ramViewer != NULL){
                mainLayout->removeWidget(ramViewer);
                delete ramViewer;
            }
            if(ramLabel != NULL){
                labelText = ramLabel->text();
                mainLayout->removeWidget(ramLabel);
                delete ramLabel;
            }
            newRamLabel(this);
            mainLayout->addWidget(ramLabel);
            ramViewer = new HexView(start, end, mac, this);
            connect(ramViewer, SIGNAL(doubleClicked(Word)), this, SLOT(inspectPointer(Word)));
            mainLayout->addWidget(ramViewer);
        }
    }
}

void ramView::inspectPointer(Word addr){
    if(startEd->text() != endEd->text() && !offset){
        endEd->setText("");
    }
    setLablelVal(startEd, addr);
    visualize();
}

void ramView::setLablelVal(QLineEdit *lab, Word val){
    lab->setText(QString::number(val, 16));
}

#endif //QARM_RAMVIEW_CC
