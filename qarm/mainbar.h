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

#ifndef QARM_MAINBAR_H
#define QARM_MAINBAR_H

#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QSlider>
#include <QMenu>

#include "facilities/arch.h"

class styledButton;

class mainBar : public QToolBar
{
    Q_OBJECT
public:
    explicit mainBar(QWidget *parent = 0);
    bool getPowerState() { return onState; }
    void poweron();
    void run();
    void showT0();
    void disableMainbar(bool value);
    bool getDisabled() { return isDisabled; }

public slots:
    void setSpeed(int speedVal);

signals:
    void play(int speedVal);
    void reset();
    void pause();
    void step();
    void speedChanged(int speedVal);
    void openRAM();
    void openBIOS();
    void showRam();
    void showConfig();
    void showTerminal(unsigned int devNo);
    void showBPW();
    void hideBPW();
    void showTLB();
    void hideTLB();
    void showSTW();
    void hideSTW();
    void powerOn();

private slots:
    void doPowerOn();
    void doPowerOff();
    void showDropDownMenu();
    void showRamMenu();
    void setSpeedLab(int speedVal);
    void playToggled(bool checked);
    void resetPressed();
    void stop();
    void plus();
    void minus();
    void updateStatus(QString state);
    void setTerminalEnabled(unsigned int devNo, bool enabled);
    void showTerminalClicked();
    void uncheckBPB();
    void toggleBPButton(bool checked);
    void uncheckTLB();
    void toggleTlbViewer(bool status);
    void uncheckSTA();
    void toggleStructsViewer(bool status);

private:
    styledButton *playB, *resetB, *stepB, *ramB, *plusB, *minusB, *configB, *windowB, *breakpB, *tlbB;   //interactions
    QVBoxLayout *plusMinusL, *scrollerL, *utilsL;
    QHBoxLayout *utilsUpperL, *utilsLowerL;
    QWidget *plusMinusW, *scrollerW, *utilsW;
    const QIcon *playIco, *pauseIco;
    QSlider *speedSl;
    QLabel *speedLab, *statusLab;
    QMenu *windowMenu, *ramMenu;
    QAction* showTerminalActions[N_DEV_PER_IL], *ramAction, *structsAction;
    bool onState;
    bool isDisabled;
};

class styledButton : public QToolButton{
    Q_OBJECT
public:
    styledButton(QWidget *parent = 0) : QToolButton(parent){
        setAutoRaise(true);
    }
};

#endif // QARM_MAINBAR_H
