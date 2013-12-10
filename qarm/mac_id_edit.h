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

#ifndef QMPS_MAC_ID_EDIT_H
#define QMPS_MAC_ID_EDIT_H

#include <QLineEdit>

#include "armProc/basic_types.h"

class MacIdEdit : public QLineEdit {
public:
    MacIdEdit(QWidget* parent = 0);

    uint8_t* getMacId(uint8_t* id) const;
    void setMacId(const uint8_t* id);
};

#endif // QMPS_MAC_ID_EDIT_H
