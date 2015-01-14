/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uMPS - A general purpose computer system simulator
 *
 * Copyright (C) 2010 Tomislav Jonjic
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

#ifndef QARM_TERMINAL_WINDOW_CC
#define QARM_TERMINAL_WINDOW_CC

#include "qarm/terminal_window.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>

#include "services/debug.h"
#include "armProc/types.h"
#include "armProc/device.h"
#include "qarm/terminal_view.h"
#include "qarm/terminal_window_priv.h"
#include "qarm/flat_push_button.h"
#include "qarm/qarm.h"

/* FIXME: this class has problems.. */

TerminalWindow::TerminalWindow(unsigned int devNo, QWidget* parent)
    : QMainWindow(parent),
      devNo(devNo),
      parent(parent)
{
    setWindowTitle(QString("uARM Terminal %1").arg(devNo));
    setWindowIcon(QIcon(LIB_PATH "icons/terminal-32.png"));

    TerminalDevice* terminal = getTerminal(devNo);

    centralWidget = new QWidget(this);

    layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    setCentralWidget(centralWidget);
    terminalView = new TerminalView(terminal, centralWidget);
    layout->addWidget(terminalView);
    statusWidget = new TerminalStatusWidget(terminal, centralWidget);
    layout->addWidget(statusWidget);

    /*STATIC: default geometry for each window for now..
    QString key = QString("TerminalWindow%1/geometry").arg(devNo);
    QVariant savedGeometry = Appl()->settings.value(key);
    if (savedGeometry.isValid()) {
        restoreGeometry(savedGeometry.toByteArray());
    } else {*/
        QFontMetrics fm = terminalView->fontMetrics();
        resize(fm.width(QLatin1Char('x')) * kDefaultCols, fm.lineSpacing() * kDefaultRows);
    //}

    connect((qarm*) parent, SIGNAL(resetMachine()), this, SLOT(onMachineReset()));
}

void TerminalWindow::closeEvent(QCloseEvent* event)
{
    /*STATIC: default geometry, as above
    QString key = QString("TerminalWindow%1/geometry").arg(devNo);
    Appl()->settings.setValue(key, saveGeometry());
    */
    event->accept();
}

void TerminalWindow::onMachineReset()
{
    delete terminalView;
    delete statusWidget;

    TerminalDevice* terminal = getTerminal(devNo);

    terminalView = new TerminalView(terminal, centralWidget);
    layout->addWidget(terminalView);

    statusWidget = new TerminalStatusWidget(terminal, centralWidget);
    layout->addWidget(statusWidget);
}

TerminalDevice* TerminalWindow::getTerminal(unsigned int devNo)
{
    Device* device = ((qarm *) parent)->getMachine()->getBus()->getDev(4, devNo);
    assert(device->Type() == TERMDEV);
    return static_cast<TerminalDevice*>(device);
}


TerminalStatusWidget::TerminalStatusWidget(TerminalDevice* t, QWidget* parent)
    : QWidget(parent),
      terminal(t),
      expanded(false),
      expandedIcon(LIB_PATH "icons/expander_down-16.png"),
      collapsedIcon(LIB_PATH "icons/expander_up-16.png")
{
    QGridLayout* layout = new QGridLayout(this);
    layout->setContentsMargins(5, 0, 5, 0);
    layout->setColumnStretch(0, 1);

    hwFailureCheckBox = new QCheckBox("Hardware Failure", this);
    hwFailureCheckBox->setAccessibleName(hwFailureCheckBox->text());
    hwFailureCheckBox->setChecked(terminal->getDevNotWorking());
    connect(hwFailureCheckBox, SIGNAL(clicked(bool)),
            this, SLOT(onHardwareFailureButtonClicked(bool)));
    layout->addWidget(hwFailureCheckBox, 0, 0);

    expanderButton = new FlatPushButton(collapsedIcon, "Show Status", this);
    expanderButton->setAccessibleName("Show Status");
    connect(expanderButton, SIGNAL(clicked()), this, SLOT(onExpanderButtonClicked()));
    expanderButton->setIconSize(QSize(16, 16));
    layout->addWidget(expanderButton, 0, 1);

    statusAreaWidget = new QWidget(this);
    QGridLayout* statusAreaLayout = new QGridLayout(statusAreaWidget);
    statusAreaLayout->setContentsMargins(0, 0, 0, 5);
    statusAreaLayout->setVerticalSpacing(5);
    statusAreaLayout->setHorizontalSpacing(15);
    statusAreaLayout->setColumnStretch(1, 1);
    statusAreaLayout->setColumnStretch(3, 1);

    statusAreaLayout->addWidget(new QLabel("RX:", statusAreaWidget), 0, 0);
    statusAreaLayout->addWidget(new QLabel("TX:", statusAreaWidget), 1, 0);

    rxStatusLabel = new QLabel(statusAreaWidget);
    rxStatusLabel->setAccessibleName("RX Status");
    rxStatusLabel->setMinimumWidth(kStatusLabelsMinimumWidth);
    rxStatusLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    statusAreaLayout->addWidget(rxStatusLabel, 0, 1);

    txStatusLabel = new QLabel(statusAreaWidget);
    txStatusLabel->setAccessibleName("TX Status");
    txStatusLabel->setMinimumWidth(kStatusLabelsMinimumWidth);
    txStatusLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    statusAreaLayout->addWidget(txStatusLabel, 1, 1);

    statusAreaLayout->addWidget(new QLabel("At:", statusAreaWidget), 0, 2);
    statusAreaLayout->addWidget(new QLabel("At:", statusAreaWidget), 1, 2);

    rxCompletionTime = new QLabel(statusAreaWidget);
    rxCompletionTime->setAccessibleName("RX Completion Time");
    rxCompletionTime->setMinimumWidth(kStatusLabelsMinimumWidth);
    rxCompletionTime->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    statusAreaLayout->addWidget(rxCompletionTime, 0, 3);

    txCompletionTime = new QLabel(statusAreaWidget);
    txCompletionTime->setAccessibleName("TX Completion Time");
    txCompletionTime->setMinimumWidth(kStatusLabelsMinimumWidth);
    txCompletionTime->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    statusAreaLayout->addWidget(txCompletionTime, 1, 3);

    layout->addWidget(statusAreaWidget, 1, 0, 1, 2);
    statusAreaWidget->hide();

    connect(terminal, SIGNAL(SignalStatusChanged(const char*)), this, SLOT(updateStatus()));
    connect(terminal, SIGNAL(SignalConditionChanged(bool)), this, SLOT(onConditionChanged(bool)));

    updateStatus();
}

void TerminalStatusWidget::updateStatus()
{
    rxStatusLabel->setText(terminal->getRXStatus());
    txStatusLabel->setText(terminal->getTXStatus());

    rxCompletionTime->setText(terminal->getRXCTimeInfo().c_str());
    txCompletionTime->setText(terminal->getTXCTimeInfo().c_str());
}

void TerminalStatusWidget::onConditionChanged(bool isWorking)
{
    hwFailureCheckBox->setChecked(!isWorking);
}

void TerminalStatusWidget::onHardwareFailureButtonClicked(bool checked)
{
    terminal->setCondition(!checked);
}

void TerminalStatusWidget::onExpanderButtonClicked()
{
    if (expanded) {
        expanded = false;
        expanderButton->setIcon(collapsedIcon);
        expanderButton->setText("Show Status");
        statusAreaWidget->hide();
    } else {
        expanded = true;
        expanderButton->setIcon(expandedIcon);
        expanderButton->setText("Hide Status");
        statusAreaWidget->show();
    }
    expanderButton->setAccessibleName(expanderButton->text());
}

#endif //QARM_TERMINAL_WINDOW_CC
