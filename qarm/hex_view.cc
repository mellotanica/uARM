/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uMPS - A general purpose computer system simulator
 *
 * Copyright (C) 2010 Tomislav Jonjic
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

#ifndef QARM_HEX_VIEW_CC
#define QARM_HEX_VIEW_CC

#include "qarm/hex_view.h"

#include <algorithm>

#include <QTextBlock>
#include <QPainter>
#include <QScrollBar>

#include "qarm/procdisplay.h"
#include "services/debug_session.h"
#include "qarm/hex_view_priv.h"

extern "C"{
#include "services/disass.h"
}

HexView::HexView(Word start, Word end, machine *mac, QWidget* parent)
    : QPlainTextEdit(parent),
      start(start),
      end(end),
      mac(mac),
      length(((end - start) >> 2) + 1),
      invalidByteRepr(QString("%1%2")
                      .arg(QChar(kInvalidLocationChar))
                      .arg(QChar(kInvalidLocationChar))),
      margin(new HexViewMargin(this))
{
    QFont font = monoLabel::getMonospaceFont();
    setFont(font);

    setViewportMargins(margin->sizeHint().width(), 0, 0, 0);
    connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(updateMargin(QRect, int)));

    setCursorWidth(fontMetrics().width(QLatin1Char('o')));
    setLineWrapMode(NoWrap);
    setOverwriteMode(true);
    setTabChangesFocus(true);
    setContextMenuPolicy(Qt::NoContextMenu);
    setUndoRedoEnabled(false);
    viewport()->setCursor(Qt::ArrowCursor);

    Refresh();
    moveCursor(QTextCursor::Start);
    highlightWord();

    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorPositionChanged()));
}

void HexView::setReversedByteOrder(bool setting)
{
    if (m_reversedByteOrder != setting) {
        m_reversedByteOrder = setting;
        Refresh();
    }
}

void HexView::Refresh(QString symbol){
    if(symbol != NULL && !symbol.isEmpty() &&
            DebuggerHolder::getInstance()->getDebugSession()->getSymbolTable() != NULL){
        const Symbol *s = DebuggerHolder::getInstance()->getDebugSession()->getSymbolTable()->Lookup(symbol.toStdString().c_str(), Symbol::TYPE_OBJECT).front();
        start = s->getStart();
        end = s->getEnd();
    }
    Refresh();
}

void HexView::Refresh()
{
    int savedPosition = textCursor().position();
    int hScrollValue = horizontalScrollBar()->value();
    int vScrollValue = verticalScrollBar()->value();

    QString buf;
    char *disass = new char[DBUFSIZE];
    buf.reserve(length * kCharsPerWord);

    for (Word addr = start; addr <= end && addr >= start; addr += WS) {
        Word data;
        unsigned int wi = (addr - start) >> 2;
        if (wi && !(wi % kWordsPerRow)){ //no more bytes to print, add disassembly
            if(data != 0 && data){
                buf += "   ";
                arm_disass(addr,data,disass);
                buf += QString(disass);
            }
            buf += '\n';
        }

        if (mac->getBus()->readW(&addr, &data) != ABT_NOABT) {
            for (unsigned int bi = 0; bi < WS; bi++) {
                if (bi > 0 || wi % kWordsPerRow)
                    buf += ' ';
                buf += invalidByteRepr;
            }
        } else {
            for (unsigned int bi = 0; bi < WS; bi++) {
                if (bi > 0 || wi % kWordsPerRow)
                    buf += ' ';
                unsigned int byteVal;
                if (m_reversedByteOrder)
                    byteVal = ((unsigned char *) &data)[WS - bi - 1];
                else
                    byteVal = ((unsigned char *) &data)[bi];
                buf += QString("%1").arg(byteVal, 2, 16, QLatin1Char('0'));
            }
        }
    }

    setPlainText(buf);

    QTextCursor cursor = textCursor();
    cursor.setPosition(savedPosition);
    setTextCursor(cursor);
    horizontalScrollBar()->setValue(hScrollValue);
    verticalScrollBar()->setValue(vScrollValue);
}

void HexView::MoveInterval(Word start, Word end){
    this->start = start;
    this->end = end;
    length = ((end - start) >> 2) + 1;

    moveCursor(QTextCursor::Start);
    highlightWord();

    Refresh();
}

void HexView::resizeEvent(QResizeEvent* event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    margin->setGeometry(QRect(cr.left(), cr.top(), margin->sizeHint().width(), cr.height()));
}

bool HexView::canInsertFromMimeData(const QMimeData* source) const
{
    UNUSED_ARG(source);
    return false;
}

void HexView::insertFromMimeData(const QMimeData* source)
{
    UNUSED_ARG(source);
}

void HexView::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
    case Qt::Key_Home:
    case Qt::Key_End:
        QPlainTextEdit::keyPressEvent(event);
        break;

    default:/*
        if (event->text().isEmpty()) {
            event->ignore();
            return;
        }
        QString digit = event->text().left(1).toLower();
        bool isHex;
        digit.toUInt(&isHex, 16);
        if (!isHex) {
            event->ignore();
            return;
        }

        unsigned int nibble = currentNibble();
        QTextCursor cursor = textCursor();
        int cp = cursor.position();
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        cursor.insertText(digit);
        cursor.setPosition(cp);
        setTextCursor(cursor);

        Word paddr = start + currentWord() * WS;
        if (debugSession->getMachine()->WriteMemory(paddr, dataAtCursor())) {
            QarmMessageBox *warning = new QarmMessageBox(QarmMessageBox::WARNING, "Warning", QString("Could not write location %1").arg(FormatAddress(paddr)).toStdString().c_str(), this);
            warning->show();
            Refresh();
        } else {
            moveCursor(QTextCursor::Right, 1 + kHorizontalSpacing * nibble);
        }*/
        break;
    }
}

void HexView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
        return;

    QTextCursor cursor = cursorForPosition(event->pos());
    setPoint(currentWord(cursor), currentByte(cursor),
             std::min(currentNibble(cursor), (unsigned int) COL_LO_NIBBLE));
}

void HexView::updateMargin(const QRect& rect, int dy)
{
    if (dy != 0)
        margin->scroll(0, dy);
    else
        margin->update(0, rect.y(), margin->width(), rect.height());
}

void HexView::onCursorPositionChanged()
{
    highlightWord();
}

unsigned int HexView::currentWord(const QTextCursor& cursor) const
{
    if (cursor.isNull())
        return textCursor().position() / kCharsPerWord;
    else
        return cursor.position() / kCharsPerWord;
}

unsigned int HexView::currentByte(const QTextCursor& cursor) const
{
    if (cursor.isNull())
        return (textCursor().position() / N_COLS_PER_BYTE) % WS;
    else
        return (cursor.position() / N_COLS_PER_BYTE) % WS;
}

unsigned int HexView::currentNibble(const QTextCursor& cursor) const
{
    if (cursor.isNull())
        return textCursor().position() % N_COLS_PER_BYTE;
    else
        return cursor.position() % N_COLS_PER_BYTE;
}

unsigned char HexView::byteValue(unsigned int word, unsigned int byte) const
{
    return toPlainText().mid(word * kCharsPerWord + N_COLS_PER_BYTE * byte, 2).toUInt(0, 16);
}

Word HexView::dataAtCursor() const
{
    Word data;

    for (unsigned int i = 0; i < WS; i++) {
        if (m_reversedByteOrder)
            ((unsigned char *) &data)[i] = byteValue(currentWord(), WS - i - 1);
        else
            ((unsigned char *) &data)[i] = byteValue(currentWord(), i);
    }

    return data;
}

void HexView::moveCursor(QTextCursor::MoveOperation operation, int n)
{
    QTextCursor cursor = textCursor();
    if (cursor.movePosition(operation, QTextCursor::MoveAnchor, n))
        setTextCursor(cursor);
}

void HexView::setPoint(unsigned int word, unsigned int byte, unsigned int nibble)
{
    //assert(byte < WS);
    //assert(nibble <= 1);

    if (word >= length)
        return;

    if (nibble > COL_LO_NIBBLE)
        nibble = COL_LO_NIBBLE;

    QTextCursor c = textCursor();
    c.setPosition(word * kCharsPerWord + byte * N_COLS_PER_BYTE + nibble);
    setTextCursor(c);
}

void HexView::paintMargin(QPaintEvent* event)
{
    QPainter painter(margin);
    const QRect& er = event->rect();

    painter.fillRect(er, palette().window().color());

    painter.setPen(palette().shadow().color());
    painter.drawLine(er.topRight(), er.bottomRight());

    painter.setPen(palette().windowText().color());

    QTextBlock block = firstVisibleBlock();
    if (!block.isValid())
        return;

    int y0 = (int) blockBoundingGeometry(block).translated(contentOffset()).y();
    int y1 = y0 + (int) blockBoundingRect(block).height();

    Word addr = start + block.blockNumber() * kWordsPerRow * WS;

    while (y0 <= er.bottom()) {
        if (er.top() <= y1) {
            painter.drawText(HexViewMargin::kLeftPadding, y0,
                             margin->width(), margin->fontMetrics().height(),
                             Qt::AlignLeft, FormatAddress(addr));
        }
        block = block.next();
        if (!block.isValid())
            break;
        y0 = y1;
        y1 += blockBoundingRect(block).height();
        addr += WS * kWordsPerRow;
    }
}

void HexView::highlightWord()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextEdit::ExtraSelection selection;

    QTextCursor cursor = textCursor();

    cursor.clearSelection();

    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor, 1);

    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                        kCharsPerWord - 1);

    selection.cursor = cursor;
    selection.format.setBackground(palette().highlight());
    selection.format.setForeground(palette().highlightedText());

    extraSelections.append(selection);
    setExtraSelections(extraSelections);
}


HexViewMargin::HexViewMargin(HexView* view)
    : QWidget(view),
      hexView(view)
{}

QSize HexViewMargin::sizeHint() const
{
    return QSize(kLeftPadding + fontMetrics().width("0xdead.beef") + kRightPadding, 0);
}

void HexViewMargin::paintEvent(QPaintEvent* event)
{
    hexView->paintMargin(event);
}

void HexViewMargin::wheelEvent(QWheelEvent* event)
{
    hexView->wheelEvent(event);
}

#endif //QARM_HEX_VIEW_CC
