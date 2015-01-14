/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uMPS - A general purpose computer system simulator
 *
 * Copyright (C) 2011 Tomislav Jonjic
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

#ifndef QARM_MAC_ID_EDIT_CC
#define QARM_MAC_ID_EDIT_CC

#include "qarm/mac_id_edit.h"

#include <QValidator>

#include "services/lang.h"
#include "services/utility.h"

class Validator : public QValidator {
public:
    Validator(QObject* parent = 0)
        : QValidator(parent) {}

    virtual State validate(QString& input, int& pos) const;
};

QValidator::State Validator::validate(QString& input, int& pos) const
{
    UNUSED_ARG(pos);

    input.replace(' ', '0');
    if (input.left(2).toUInt(0, 16) % 2)
        return Invalid;
    else
        return Acceptable;

}


MacIdEdit::MacIdEdit(QWidget* parent)
    : QLineEdit(parent)
{
    setText("00:00:00:00:00:00");
    setInputMask("HH:HH:HH:HH:HH:HH");
    setValidator(new Validator(parent));
}

uint8_t* MacIdEdit::getMacId(uint8_t* id) const
{
    return ParseMACId(qPrintable(text()), id);
}

void MacIdEdit::setMacId(const uint8_t* id)
{
    setText(MACIdToString(id).c_str());
}

#endif //QARM_MAC_ID_EDIT_CC
