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

#ifndef QMPS_TERMINAL_VIEW_H
#define QMPS_TERMINAL_VIEW_H

#include <QPlainTextEdit>
#include <QByteArray>

class TerminalDevice;

class TerminalView : public QPlainTextEdit
{
    Q_OBJECT

public:
    TerminalView(TerminalDevice* terminal, QWidget* parent = 0);

protected:
    virtual void keyPressEvent(QKeyEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);

    virtual bool canInsertFromMimeData(const QMimeData* source) const;
    virtual void insertFromMimeData(const QMimeData* source);

private:
    void flushInput();
    void onCharTransmitted(char c);

    TerminalDevice* const terminal;
    QByteArray input;
};

#endif // QMPS_TERMINAL_VIEW_H
