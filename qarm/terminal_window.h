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

#ifndef QARM_TERMINAL_WINDOW_H
#define QARM_TERMINAL_WINDOW_H

#include <QMainWindow>
#include <QApplication>

class TerminalDevice;
class QLabel;
class QVBoxLayout;
class TerminalView;
class TerminalStatusWidget;

class TerminalWindow : public QMainWindow {
    Q_OBJECT

public:
    TerminalWindow(unsigned int devNo, QWidget* parent = 0);

protected:
    virtual void closeEvent(QCloseEvent* event);

private slots:
    void onMachineReset();

private:
    static const int kDefaultCols = 60;
    static const int kDefaultRows = 20;

    TerminalDevice* getTerminal(unsigned int devNo);

    const unsigned int devNo;

    QWidget *parent;
    QVBoxLayout* layout;
    TerminalView* terminalView;
    TerminalStatusWidget* statusWidget;
};

#endif // QARM_TERMINAL_WINDOW_H
