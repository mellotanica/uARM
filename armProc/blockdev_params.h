/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uMPS - A general purpose computer system simulator
 *
 * Copyright (C) 2004 Mauro Morsiani
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

/****************************************************************************
 * 
 * This module contains constant & macro definitions useful for block devices
 * handling and for file identification.
 *
 ****************************************************************************/

// file recognition tags (magic numbers) 
#define DISKFILEID	0x0053504D 
#define TAPEFILEID	0x0153504D
#define BIOSFILEID	0x0253504D
#define COREFILEID	0x0353504D
#define AOUTFILEID	0x0453504D
#define STABFILEID	0x4153504D

// tape markers
#define TAPESTART	3
#define TAPEEOB	2
#define TAPEEOF	1 
#define TAPEEOT	0


// DriveParams class items constants: position, min, max and default (DFL)
// values (where applicable) are given for each: see class definition

// number of parameters
#define DRIVEPNUM	6

// number of cylinders: 2 bytes (64 K)
#define CYLNUM	0
#define MAXCYL	0xFFFF
#define DFLCYL	32

// number of heads: 1 byte
#define HEADNUM	1
#define MAXHEAD	0xFF
#define DFLHEAD	2


// number of sectors: 1 byte
#define SECTNUM	2
#define MAXSECT	0xFF
#define DFLSECT	8

// 1 full rotation (in microseconds): here min and max values are given
#define ROTTIME	3
#define MINRPM 	360
#define MAXRPM	10800
#define DFLROTTIME 16666

// average track-to-track seek time in microseconds
#define SEEKTIME	4
#define MAXSEEKTIME	10000
#define DFLSEEKTIME	100

// data percentage of sector: 1-100 (min and max values given)
#define DATASECT	5
#define MINDATAS	10
#define MAXDATAS	90
#define DFLDATAS	80
