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

#ifndef QMPS_HEX_VIEW_H
#define QMPS_HEX_VIEW_H

#include <QPlainTextEdit>

//#include "umps/arch.h"
//#include "umps/stoppoint.h"
//#include "qmps/memory_view_delegate.h"

#include "qarm/guiConst.h"
#include "armProc/machine.h"

class HexViewMargin;

class HexView : public QPlainTextEdit
{
    Q_OBJECT
    Q_PROPERTY(bool reversedByteOrder READ HasReversedByteOrder WRITE setReversedByteOrder)

public:
    enum ByteOrder {
        BYTE_ORDER_NATIVE,
        BYTE_ORDER_REVERSED
    };

    HexView(Word start, Word end, machine *mac, QWidget* parent = 0);

    bool HasReversedByteOrder() const { return revByteOrder; }
    void setReversedByteOrder(bool setting);

public slots:
    void Refresh();

protected:
    void resizeEvent(QResizeEvent* event);

    bool canInsertFromMimeData(const QMimeData* source) const;
    void insertFromMimeData(const QMimeData* source);

    void keyPressEvent(QKeyEvent* event);
    void mousePressEvent(QMouseEvent* event);

private slots:
    void updateMargin(const QRect& rect, int dy);
    void onCursorPositionChanged();

private:
    enum {
        COL_HI_NIBBLE    = 0,
        COL_LO_NIBBLE    = 1,
        COL_SPACING      = 2,
        N_COLS_PER_BYTE
    };

    static const unsigned int kWordsPerRow = 2;
    static const unsigned int kCharsPerWord = WS * N_COLS_PER_BYTE;
    static const unsigned int kCharsPerRow = kWordsPerRow * kCharsPerWord;
    static const unsigned int kHorizontalSpacing = 1;

    static const unsigned int kInvalidLocationChar = 0x2592;

    unsigned int currentWord(const QTextCursor& cursor = QTextCursor()) const;
    unsigned int currentByte(const QTextCursor& cursor = QTextCursor()) const;
    unsigned int currentNibble(const QTextCursor& cursor = QTextCursor()) const;

    unsigned char byteValue(unsigned int word, unsigned int byte) const;
    Word dataAtCursor() const;

    void moveCursor(QTextCursor::MoveOperation operation, int n = 1);
    void setPoint(unsigned int word,
                  unsigned int byte = 0,
                  unsigned int nibble = 0);

    void paintMargin(QPaintEvent* event);
    void highlightWord();

    friend class HexViewMargin;

    machine *mac;

    const Word start;
    const Word end;
    const Word length;

    bool revByteOrder;

    const QString invalidByteRepr;

    HexViewMargin* margin;

    QString FormatAddress(Word addr)
    {
        return (QString("0x%1.%2")
                .arg((quint32) (addr >> 16), 4, 16, QChar('0'))
                .arg((quint32) (addr & 0x0000ffffU), 4, 16, QChar('0')));
    }
};

#endif // QMPS_HEX_VIEW_H
