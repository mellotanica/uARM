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
 * This header file contains registers constans & definitions.
 */

#ifndef UARM_REGISTERS_H
#define UARM_REGISTERS_H

/* 
 * CPU registers: 
 * 
 * 
 * 0-15 -> r0-r15 
 * 16 -> cpsr
 * 17-23, 24 -> r8_fiq-r14_fiq, spsr_fiq
 * 25-26, 27 -> r13_svc-r14_svc, spsr_svc
 * 28-29, 30 -> r13_abt-r14_abt, spsr_abt
 * 31-32, 33 -> r13_irq-r14_irq, spsr_irq
 * 34-35, 36 -> r13_undef-r14_undef, spsr_undef
 */

#define CPU_REGISTERS_NUM 37

#define REG_CPSR 16	//current program status register
#define REG_SPSR 17	//saved program status register
#define REG_PC 15	//program counter
#define REG_SP 13	//stack pointer
#define REG_LR 14	//link return
#define REG_FP 11	//frame pointer

#define REG_LR_FIQ 23
#define REG_SPSR_FIQ 24

#define REG_LR_SVC 26
#define REG_SPSR_SVC 27

#define REG_LR_ABT 29
#define REG_SPSR_ABT 30

#define REG_LR_IRQ 32
#define REG_SPSR_IRQ 33

#define REG_LR_UND 35
#define REG_SPSR_UND 36

//register maps
#define REG_FIQ_BASE 17
#define REG_SVC_BASE 25
#define REG_ABT_BASE 28
#define REG_IRQ_BASE 31
#define REG_UNDEF_BASE 34

//status register structure
#define MODE_POS 0
#define T_POS 5
#define F_POS 6
#define I_POS 7
#define V_POS 28
#define C_POS 29
#define Z_POS 30
#define N_POS 31

//#define A_POS 8
//#define E_POS 9
//#define GE_POS 16
//#define J_POS 24
//#define Q_POS 27


//status register masks
#define MODE_MASK (0x1F<<MODE_POS)	//processor mode
#define T_MASK (1<<T_POS)		//thumb isa on
#define F_MASK (1<<F_POS)		//disable fast interrupts
#define I_MASK (1<<I_POS)		//disable interrupts
#define V_MASK (1<<V_POS)		//operation overflow
#define C_MASK (1<<C_POS)		//operation carried out
#define Z_MASK (1<<Z_POS)		//zero result
#define N_MASK (1<<N_POS)		//negative result
#define RESERVED_MASK 0x0FFFFF00
#define PRIVILEGED_MASK ((MODE_MASK|T_MASK|F_MASK|I_MASK))

//#define A_MASK (1<<A_POS)		//disable imprecise data abort
//#define E_MASK (1<<E_POS)		//load-store endianess
//#define GE_MASK (0xF<<GE_POS)	//greater than or equal
//#define J_MASK (1<<J_POS)		//jazelle isa on
//#define Q_MASK (1<<Q_POS)		//DSP-instruction overflow/saturation
//#define PRIVILEGED_MASK ((MODE_MASK|T_MASK|F_MASK|A_MASK|I_MASK|J_MASK))
//#define RESERVED_MASK ((0x3F<<10)|(0xF<<20)|(3<<25))	//reserved bits (0x6F0FC00)


/*
 * CP15 registers
 */
 
#define CP15_REGISTERS_NUM 17

#define REG_READ 0
#define REG_WRITE 1

#endif //UARM_REGISTERS_H
