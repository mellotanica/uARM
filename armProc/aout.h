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

/*
 * This header file contains constant & macro definitions for .aout/.core 
 * file types.
 */

#ifndef UMPS_AOUT_H
#define UMPS_AOUT_H

/* Number of BIOS reserved page frames */
#define N_BIOS_PAGES  1

/*
 * Core file header size in words: core file id tag (1W) + 1 full page frame
 * for BIOS exclusive use
 */
#define CORE_HDR_SIZE (N_BIOS_PAGES * 1024 + 1)

/*
 * AOUT header entries
 */
#define AOUT_HE_TAG           0
#define AOUT_HE_ENTRY         1
#define AOUT_HE_TEXT_VADDR    2
#define AOUT_HE_TEXT_MEMSZ    3
#define AOUT_HE_TEXT_OFFSET   4
#define AOUT_HE_TEXT_FILESZ   5
#define AOUT_HE_DATA_VADDR    6
#define AOUT_HE_DATA_MEMSZ    7
#define AOUT_HE_DATA_OFFSET   8
#define AOUT_HE_DATA_FILESZ   9
#define AOUT_HE_GP_VALUE      42

#define N_AOUT_HDR_ENT     43

#endif /* !defined(UMPS_AOUT_H) */
