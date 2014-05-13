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

/*
 * This header file contains the global constant & macro definitions.
 */

#ifndef UARM_CONST_H
#define UARM_CONST_H

#include "config.h"

#include "armProc/registers.h"
#include "armProc/types.h"
#include <math.h>
#include <stdlib.h>

#define DEBUG_ON

#ifdef DEBUG_ON
	#include <iostream>
	using namespace std;
#endif

#define COPROCESSORS_NUM 16

/*
 * comment/uncomment lines to disable/enable coprocessors
 * coprocessor lines other than 4,5,6,7,14 and 15 are reserved
 * 
 * each coprocessor must be a an object of the class cpN where N is the coprocessor line number
 * cpN class must be and implementation of coprocessor interface 
 * (with explicit fetch() decode() and execute() implementations)
 */
#define COPROCESSOR_CP15 15
//#define COPROCESSOR_CP14 14
//#define COPROCESSOR_CP7 7
//#define COPROCESSOR_CP6 6
//#define COPROCESSOR_CP5 5
//#define COPROCESSOR_CP4 4


#define ENDIANESS_BIGENDIAN false	//selects endianess used to communicate with the memory subsystem

#define OP_HALT 0xFFFFFFFF	//fake opcode that represets the end of a program

#define BX_MASK ((1<<4)|(0xFFF<<8)|(1<<21)|(1<<24))
#define NBX_MASK ((7<<5)|(1<<20)|(3<<22)|(7<<25))
#define SWP_MASK ((1<<4)|(1<<7)|(1<<24))
#define NSWP_MASK ((3<<5)|(0xF<<8)|(3<<20)|(1<<23)|(7<<25))
#define MUL_MASK ((1<<4)|(1<<7))
#define NMUL_MASK ((3<<5)|(0x3F<<22))
#define LMUL_MASK ((1<<4)|(1<<7)|(1<<23))
#define NLMUL_MASK ((3<<5)|(0xF<<24))
#define HWT_MASK ((1<<4)|(1<<7))
#define NHWT_MASK ((0xF<<8)|(1<<22)|(7<<25))
#define HWTI_MASK ((1<<4)|(1<<7)|(1<<22))
#define NHWTI_MASK (7<<25)


#define PSR_UNALLOC_MASK	0x0FFFFF00
#define PSR_USER_MASK		0xF0000000
#define PSR_PRIV_MASK		0x000000DF
#define PSR_STATE_MASK		0x00000020

//#define MEM_SIZE_B 4294967296 //2^32
#define MEM_SIZE_W 10485760 // 10MB
//#define MEM_SIZE_B 2147483648 //2^31
#define MEM_SIZE_B 1073741824 //2^30
#define MEM_SIZE_H 536870912 //2^29
//#define MEM_SIZE_W 268435456 //2^28

#define BYTES_PER_FRAME 4096

#define SIGNMASK	0x80000000UL

#define VM_PSEG_START   0x00008000
#define VM_PSEG_TOP     0xC0000000
#define VM_SHSEG_START  0xC0000000
#define VM_SHSEG_TOP    0xFFFFFFFF

#define PAGE_TBL_MAGICNO    0x2A
#define PAGE_TBL_INVALID_H  0xFF0000
#define PAGE_TBL_NOT_FOUND  0xFF0001

#define PROG_START 0x20000000	//starting pc..
#define INITIAL_BRANCH (0xE12FFF10 + REG_LR)

#define PIPELINE_STAGES 3
#define PIPELINE_FETCH 0
#define PIPELINE_DECODE 1
#define PIPELINE_EXECUTE 2

#define SHIFT_LSL 0
#define SHIFT_LSR 1
#define SHIFT_ASR 2
#define SHIFT_ROR 3

#define INVERT_W(b)	((b)^0xFFFFFFFF)
#define INVERT_B(b) ((b)^1)

#ifndef NULL
#define NULL 0
#endif

/* ROM ADDRESSES */
#define ROMF_EXCVBASE   0x00007000
#define ROMF_EXCVTOP    0x00007500

#define ROMF_SEGTBASE   0x00007600
#define ROMF_SEGTTOP    0x00007E00

#define ROMF_STACKBASE  0x00007FF0
#define ROMF_STACKTOP   0x00008000



// interrupt handling related constants

#define CAUSE_IP_MASK          0xff000000
#define CAUSE_IP(line)         (1U << (24 + (line)))
#define CAUSE_IP_BIT(line)     (24 + (line))

// device starting interrupt line
#define DEVINTBASE	3

// device register length
#define DEVREGLEN 	4

// interrupts available for registers
#define DEVINTUSED 	5

// devices per interrupt line
#define DEVPERINT	8

// device type codes
#define NULLDEV	0
#define DISKDEV	1
#define TAPEDEV	2
#define ETHDEV 3
#define PRNTDEV 4
#define TERMDEV 5

// interrupt line offset used for terminals
// (lots of code must be modified if this changes)

#define TERMINT	4

// memory access types for brkpt/susp/trace ranges in watch.cc and appforms.cc
// modules
#define READWRITE 0x6
#define READ	0x4
#define WRITE	0x2
#define EXEC	0x1
#define EMPTY 	0x0

// some useful macros

// DMA transfer time
#define DMATICKS	BLOCKSIZE

// word alignment mask
#define ALIGNMASK	0x00000003UL

// recognizes bad (unaligned) virtual address
#define BADADDR(w)	((w & ALIGNMASK) != 0UL)

// returns the sign bit of a word
#define SIGNBIT(w)	(w & SIGNMASK)

// returns 1 if the two strings are equal, 0 otherwise
#define SAMESTRING(s,t)	(strcmp(s,t) == 0)

// returns 1 if a is in open-ended interval [b, c[, 0 otherwise
#define INBOUNDS(a,b,c)		(a >= b && a < c)

// word length in bytes, byte length in bits, sign masks, etc.
#define WORDLEN 4
#define BYTELEN	8
#define WORDSHIFT	2
#define MAXWORDVAL	0xFFFFFFFFUL
#define SIGNMASK	0x80000000UL
#define BYTEMASK	0x000000FFUL

// immediate/lower halfword part mask
#define IMMMASK	0x0000FFFFUL

// halfword bit length
#define HWORDLEN	16

// some utility constants
#define	HIDDEN	static

#define	EOS	'\0'
#define EMPTYSTR	""
#define	EXIT_FAILURE	1
#define	EXIT_SUCCESS	0

#define UI_UPDATE_THRESHOLD  50

// host specific constants
#ifdef WORDS_BIGENDIAN
#define BIGENDIANCPU	1
#else
#define BIGENDIANCPU	0
#endif

// hardware constants

// physical memory page frame size (in words)
#define FRAMESIZE	1024

// KB per frame
#define FRAMEKB	4

// number of ASIDs
#define MAXASID 256

// block device size in words
#define BLOCKSIZE	FRAMESIZE

// eth packet size
#define PACKETSIZE 1514

// DMA transfer time
#define DMATICKS	BLOCKSIZE

#define WAITCPINSTR 0xEEF00F21 // CDP p15, 0xF, c0, c0, c1, 1
#define HALTCPINSTR 0xEEF00F01 // CDP p15, 0xF, c0, c0, c1, 0

enum AbortType {
    ABT_NOABT   = 0,
    ABT_MEMERR  = 1,
    ABT_BUSERR  = 2,
    ABT_ADDRERR = 3,
    ABT_SEGERR  = 4,
    ABT_PAGEERR = 5,
    NOABT_ROM   = 0xFF
};


#endif //UARM_CONST_H
