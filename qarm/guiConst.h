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

#ifndef QARM_GUICONST_H
#define QARM_GUICONST_H

#define DEFAULT_CONFIG_FILE "machine.uarm.cfg"
#define DEFAULT_CONFIG_PATH "uarm"

#ifdef MACOS_BUILD
#define LIB_PATH "/usr/local/lib/uarm/"
#else
#define LIB_PATH "/usr/lib/uarm/"
#endif

#define PIPECOLS    6
#define CPUCOLS     8
#define CPUROWS     19
#define CP15COLS    5
#define CP15ROWS    5
#define INFOCOLS    3
#define INFOROWS    5

//#define WS  (sizeof(Word))
#define UNUSED_ARG(x) ((void) x)

/* Emulation speed */
#define IPSMAX      60
#define IPSTRESH    50
#define IPSMIN      1
#define IPSSTEP     1

//maximum dimension for single ram segment to show in ramviewer
#define MAX_VIEWABLE_RAM 2500

#endif // QARM_GUICONST_H
