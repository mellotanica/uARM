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

#include "registers.h"
#include "types.h"
#include <math.h>

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
#define PSR_PRIV_MASK		0x0000000F
#define PSR_STATE_MASK		0x00000020

//#define MEM_SIZE_B 4294967296 //2^32
//#define MEM_SIZE_W 10485760 // 10MB
#define MEM_SIZE_B 2147483648 //2^31
#define MEM_SIZE_H 1073741824 //2^30
#define MEM_SIZE_W 536870912 //2^29
//#define MEM_SIZE_W 268435456 //2^28

#define PIPELINE_STAGES 3
#define PIPELINE_FETCH 0
#define PIPELINE_DECODE 1
#define PIPELINE_EXECUTE 2

#define INVERT_W(b)	((b)^0xFFFFFFFF)
#define INVERT_B(b) ((b)^1)

#ifndef NULL
#define NULL 0
#endif

#endif //UARM_CONST_H
