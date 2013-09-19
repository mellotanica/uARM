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

#ifndef MAINBAR_H
#define MAINBAR_H

#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QSlider>

class styledButton;

class mainBar : public QToolBar
{
    Q_OBJECT
public:
    explicit mainBar(QWidget *parent = 0);

signals:
    void play(int speedVal);
    void reset();
    void pause();
    void step();
    void speedChanged(int speedVal);
    void open(QString file);
    void showRam();

private slots:
    void setSpeedLab(int speedVal);
    void playToggled(bool checked);
    void resetPressed();
    void stop();
    void plus();
    void minus();
    void openPressed();
    void updateStatus(QString state);

private:
    styledButton *playB, *resetB, *stepB, *openB, *ramB, *plusB, *minusB;   //interactions
    QVBoxLayout *plusMinusL, *scrollerL;
    QWidget *plusMinusW, *scrollerW;
    const QIcon *playIco, *resetIco, *pauseIco, *stepIco;
    QSlider *speedSl;
    QLabel *speedLab, *statusLab;
};

class styledButton : public QToolButton{
    Q_OBJECT
public:
    styledButton(QWidget *parent = 0) : QToolButton(parent){
        setAutoRaise(true);
    }
};

#endif // MAINBAR_H
