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

#include "qarm/flat_push_button.h"

#include "services/lang.h"

FlatPushButton::FlatPushButton(QWidget* parent)
    : QPushButton(parent)
{
    setFlat(true);
}

FlatPushButton::FlatPushButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent)
{
    setFlat(true);
}

FlatPushButton::FlatPushButton(const QIcon& icon, const QString& text, QWidget* parent)
    : QPushButton(icon, text, parent)
{
    setFlat(true);
}

void FlatPushButton::enterEvent(QEvent* event)
{
    UNUSED_ARG(event);
    setFlat(false);
}

void FlatPushButton::leaveEvent(QEvent* event)
{
    UNUSED_ARG(event);
    setFlat(true);
}
