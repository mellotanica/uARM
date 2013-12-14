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

#ifndef QARM_TERMINAL_VIEW_CC
#define QARM_TERMINAL_VIEW_CC

#include "qarm/terminal_view.h"

#include <string>

#include <QFontMetrics>
#include <QFile>
#include <QTextStream>

#include "services/lang.h"
#include "services/debug.h"
#include "armProc/device.h"
#include "qarm/procdisplay.h"
#include "armProc/machine_config.h"
//EDIT: needs app?
//#include "qarm/application.h"

TerminalView::TerminalView(TerminalDevice* terminal, QWidget* parent)
    : QPlainTextEdit(parent),
      terminal(terminal)
{
    //EDIT: sigc++
    //terminal->SignalTransmitted.connect(sigc::mem_fun(this, &TerminalView::onCharTransmitted));
    connect(terminal, SIGNAL(SignalTransmitted(char)), this, SLOT(onCharTransmitted(char)));

    QFont font = monoLabel::getMonospaceFont();
    setFont(font);
    setCursorWidth(fontMetrics().width(QLatin1Char('o')));

    // Disable features that look silly in a basic terminal widget
    setContextMenuPolicy(Qt::NoContextMenu);
    setUndoRedoEnabled(false);

    // Wrapping
    setWordWrapMode(QTextOption::WrapAnywhere);

    std::string devFile = MC_Holder::getInstance()->getConfig()->getDeviceFile(terminal->getInterruptLine(),
                                                                               terminal->getNumber());
    QFile file(devFile.c_str());
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        // TODO: fix this and another gazillion critical points
        assert(0);
    }
    QTextStream in(&file);
    setPlainText(in.readAll());

    moveCursor(QTextCursor::End);
}

void TerminalView::keyPressEvent(QKeyEvent* e)
{
    if (e->modifiers() & ~Qt::ShiftModifier)
        return;

    int key = e->key();
    if (key == Qt::Key_Return || key == Qt::Key_Enter) {
        flushInput();
        QPlainTextEdit::keyPressEvent(e);
    } else if (key == Qt::Key_Backspace && !input.isEmpty()) {
        input.chop(1);
        QPlainTextEdit::keyPressEvent(e);
    } else if (!e->text().isEmpty() && Qt::Key_Space <= key && key <= Qt::Key_nobreakspace) {
        input.append(e->text().toAscii());
        QPlainTextEdit::keyPressEvent(e);
    }
}

void TerminalView::mousePressEvent(QMouseEvent* e)
{
    UNUSED_ARG(e);
}

bool TerminalView::canInsertFromMimeData(const QMimeData* source) const
{
    UNUSED_ARG(source);
    return false;
}

void TerminalView::insertFromMimeData(const QMimeData* source)
{
    // It seems that Qt forces us to reimplement (stub out) this if we
    // want to avoid drop and clipboard paste ops in all cases.
    UNUSED_ARG(source);
}

void TerminalView::flushInput()
{
    terminal->Input(input.constData());
    input.clear();
}

void TerminalView::onCharTransmitted(char c)
{
    insertPlainText(QString(c));

    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
}

#endif //QARM_TERMINAL_VIEW_CC
