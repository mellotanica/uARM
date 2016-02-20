/*
 * uARM
 *
 * Copyright (C) 2014 Marco Melletti
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

#include "arch.h"

#ifndef BIOS_CONST_H
#define BIOS_CONST_H

.equ BIOS_SRV_SYS, 8
.equ BIOS_SRV_BP, 9
.equ BIOS_SRV_HALT, 1
.equ BIOS_SRV_PANIC, 2
.equ BIOS_SRV_LDST, 3
.equ BIOS_SRV_WAIT, 4

.equ BPEXCEPTION_CODE, 7 /* see uARMconst.h */
.equ SYSEXCEPTION_CODE, 4

.equ PSR_OFFSET, 64
.equ TIMER_INFO, 0x2E4      /* see arch.h */
.equ RAMSIZE_INFO, 0x2D4
.equ RAMBASE_INFO, 0x2D0
.equ TOD_HI_INFO, 0x2DC
.equ TOD_LO_INFO, 0x2E0

#endif // BIOS_CONST_H
