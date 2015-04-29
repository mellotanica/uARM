/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uARM
 *
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

#ifndef BASE_ERROR_CC
#define BASE_ERROR_CC

#include "qarm/qarmmessagebox.h"
#include "armProc/machine_config.h"
#include "qarm/qarm.h"
#include <QObject>

static bool alreadyPanicing = false;

void Panic(const char* message)
{
    if(!alreadyPanicing){
        alreadyPanicing = true;

        QarmMessageBox *error = new QarmMessageBox(QarmMessageBox::CRITICAL, "PANIC", QString("PANIC: %1").arg(message).toStdString().c_str());
        error->show();

        MC_Holder::getInstance()->getConfig()->getMainWidget()->stop();
        MC_Holder::getInstance()->getConfig()->getMainWidget()->setDisabled(true);

        printf("uARM PANIC: %s\n", message);

        error->connect(error, SIGNAL(accepted()), MC_Holder::getInstance()->getConfig()->getApp(), SLOT(quit()));
    }
}

#endif //BASE_ERROR_CC
