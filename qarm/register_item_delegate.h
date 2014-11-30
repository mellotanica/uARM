/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uMPS - A general purpose computer system simulator
 *
 * Copyright (C) 2010, 2011 Tomislav Jonjic
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

#ifndef QMPS_REGISTER_ITEM_DELEGATE_H
#define QMPS_REGISTER_ITEM_DELEGATE_H

#include <QStyledItemDelegate>

#include "armProc/types.h"

class RegisterItemDelegate : public QStyledItemDelegate {
public:
    RegisterItemDelegate(QObject* parent = 0)
        : QStyledItemDelegate(parent) {}

    virtual QString displayText(const QVariant& value, const QLocale& locale) const;

    virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;

    virtual void updateEditorGeometry(QWidget* editor,
                                      const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const;

protected:
    virtual QString Text(Word value) const = 0;
};

class RIDelegateHex : public RegisterItemDelegate {
public:
    RIDelegateHex(QObject* parent = 0)
        : RegisterItemDelegate(parent) {}

    virtual QWidget* createEditor(QWidget* parent,
                                  const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const;

    virtual void setModelData(QWidget* editor,
                              QAbstractItemModel* model,
                              const QModelIndex& index) const;

protected:
    virtual QString Text(Word value) const
    {
        return QString("0x%1").arg(value, 8, 16, QLatin1Char('0'));
    }
};

class RIDelegateSignedDecimal : public RegisterItemDelegate {
public:
    RIDelegateSignedDecimal(QObject* parent = 0)
        : RegisterItemDelegate(parent) {}

    virtual QWidget* createEditor(QWidget* parent,
                                  const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const;

    virtual void setModelData(QWidget* editor,
                              QAbstractItemModel* model,
                              const QModelIndex& index) const;

protected:
    virtual QString Text(Word value) const
    {
        return QString::number((SWord) value, 10);
    }
};

class RIDelegateUnsignedDecimal : public RegisterItemDelegate {
public:
    RIDelegateUnsignedDecimal(QObject* parent = 0)
        : RegisterItemDelegate(parent) {}

    virtual QWidget* createEditor(QWidget* parent,
                                  const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const;

    virtual void setModelData(QWidget* editor,
                              QAbstractItemModel* model,
                              const QModelIndex& index) const;

protected:
    virtual QString Text(Word value) const
    {
        return QString::number(value, 10);
    }
};

class RIDelegateBinary : public RegisterItemDelegate {
public:
    RIDelegateBinary(QObject* parent = 0)
        : RegisterItemDelegate(parent) {}

    virtual QWidget* createEditor(QWidget* parent,
                                  const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const;

    virtual void setModelData(QWidget* editor,
                              QAbstractItemModel* model,
                              const QModelIndex& index) const;

protected:
    virtual QString Text(Word value) const;
};

#endif // QMPS_REGISTER_ITEM_DELEGATE_H
