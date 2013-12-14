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

#ifndef QMPS_TERMINAL_WINDOW_PRIV_H
#define QMPS_TERMINAL_WINDOW_PRIV_H

#include <QWidget>
#include <QIcon>

class QLabel;
class QPushButton;
class TerminalDevice;
class QCheckBox;

class TerminalStatusWidget : public QWidget
{
    Q_OBJECT

public:
    TerminalStatusWidget(TerminalDevice* terminal, QWidget* parent = 0);

private:
    // We _have_ to set the minimumSize() property on dynamic labels
    // in resizable containers; the constant is here mainly as a
    // remainder for that :-)
    static const int kStatusLabelsMinimumWidth = 16;

    void updateStatus();
    void onConditionChanged(bool isWorking);

    TerminalDevice* const terminal;

    bool expanded;
    QWidget* statusAreaWidget;

    QLabel* rxStatusLabel;
    QLabel* rxCompletionTime;
    QLabel* txStatusLabel;
    QLabel* txCompletionTime;

    QIcon expandedIcon;
    QIcon collapsedIcon;

    QPushButton* expanderButton;

    QCheckBox* hwFailureCheckBox;

private Q_SLOTS:
    void onHardwareFailureButtonClicked(bool checked);
    void onExpanderButtonClicked();
};

#endif // QMPS_TERMINAL_WINDOW_PRIV_H
