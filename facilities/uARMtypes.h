/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
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

#ifndef UARMTYPES_H
#define UARMTYPES_H

#include <uARMconst.h>

/************************** Device register types ***************************/

/* Device register type for disks, tapes and printers (dtp) */
typedef struct {
  unsigned int status;
    unsigned int command;
    unsigned int data0;
    unsigned int data1;
} dtpreg_t;

/* Device register type for terminals; fields have different meanings */
typedef struct {
  unsigned int recv_status;
    unsigned int recv_command;
    unsigned int transm_status;
    unsigned int transm_command;
} termreg_t;

/* With a pointer to devreg_t we can refer to a "generic" device register, and
   then use the appropriate union member to access the fields (not strictly
     necessary, but for convenience and clarity) */
typedef union {
  dtpreg_t dtp;
    termreg_t term;
} devreg_t;

/************************* CPU state type ***********************************/

typedef struct{
    unsigned int a1;    //r0
    unsigned int a2;    //r1
    unsigned int a3;    //r2
    unsigned int a4;    //r3
    unsigned int v1;    //r4
    unsigned int v2;    //r5
    unsigned int v3;    //r6
    unsigned int v4;    //r7
    unsigned int v5;    //r8
    unsigned int v6;    //r9
    unsigned int sl;    //r10
    unsigned int fp;    //r11
    unsigned int ip;    //r12
    unsigned int sp;    //r13
    unsigned int lr;    //r14
    unsigned int pc;    //r15
    unsigned int cpsr;
    unsigned int CP15_Control;
    unsigned int CP15_EntryHi;
    unsigned int CP15_Cause;
    unsigned int TOD_Hi;
    unsigned int TOD_Low;
}state_t;

/********************* disk devices informations *****************************/


typedef struct {
  unsigned int max_cyl;
  unsigned int max_head;
  unsigned int max_sect;
} disk_data_t;

/********************************* ELF files ***********************************/

typedef struct {
	unsigned int tag;
	unsigned int entry_point;
	unsigned int text_vaddr;
	unsigned int text_memsz;
	unsigned int text_offset;
	unsigned int text_filesz;
	unsigned int data_vaddr;
	unsigned int data_memsz;
	unsigned int data_offset;
	unsigned int data_filesz;
} elf_hdr_s;

#endif // UARMTYPES_H
