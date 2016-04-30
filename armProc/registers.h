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
#define VM_POS 18   //if 1 virtual memory is on
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
#define C_MASK (1<<C_POS)		//operation carried out (or borrow or extend)
#define Z_MASK (1<<Z_POS)		//zero result
#define N_MASK (1<<N_POS)		//negative result (or less than)
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

#define CP15_REGISTERS_NUM 9

#define CP15_REG0_IDC    0   // ID codes (processor, cache, tightly coupled memory and TLB)
#define CP15_REG1_SCB    1   // System Control Register
#define CP15_REG1_CCB    2   // Coprocessor Access Register
/*
#define CP15_REG2_PTC    3   // Page Table Control
#define CP15_REG3_DAC    4   // Domain Access Control
#define CP15_REG5_FS     5   // Fault Status
#define CP15_REG7_CWBC   7   // Cache/Write Buffer Control
#define CP15_REG9_CL     9   // Cache Lockdown
#define CP15_REG11_DMAC  11  // DMA Control
#define CP15_REG13_PID   12  // Process ID
*/
#define CP15_REG2_EntryHi 3     // PTE Hy Word
#define CP15_REG2_EntryLo 4     // PTE Lo Word
#define CP15_REG6_FA      5     // Fault Address
#define CP15_REG8_TLBR    6     // TLB Control (TLB.Random)
#define CP15_REG10_TLBI   7     // TLB Lockdown (TLB.Index)
#define CP15_REG15_CAUSE  8     // Exception Cause

#define CP15_REG1_MPOS  0   // enable MMU or protection unit (read 0)
#define CP15_REG1_APOS  1   // enable memory alignement fault check (read 0 if arch checks memory alignement)
#define CP15_REG1_CPOS  2   // enable L1 unified/data cache (read 0 if no L1 cache, 1 if L1 cannot be disabled)
#define CP15_REG1_WPOS  3   // enable write buffer (read 0 if no WB, 1 if WB cannot be disabled)
#define CP15_REG1_BPOS  7   // set Big endian mode (must be set after reset)
#define CP15_REG1_SPOS  8   // enable System protection
#define CP15_REG1_RPOS  9   // enable ROM protection
#define CP15_REG1_FPOS  10  // Implementation defined
#define CP15_REG1_ZPOS  11  // enable program flow prediction (read 0 if no branch prediction, 1 if BP cannot be disabled)
#define CP15_REG1_IPOS  12  // enable L1 instruction cache (read as C bit)
#define CP15_REG1_VPOS  13  // set hi exception vectors (must be set after reset)
#define CP15_REG1_RRPOS 14  // enable cache predictable replacement strategy (read 0)
#define CP15_REG1_L4POS 15  // inhibits thumb (bit[0] doesn't update the T bit in CPSR) see LDM, LDR, POP
#define CP15_REG1_FIPOS 21  // disable implementation defined performance feats for FIQ
#define CP15_REG1_UPOS  22  // enables unaligned and mixed-edned data loading (should be 0)
#define CP15_REG1_XPPOS 23  // disables subpage AP bits for page table
#define CP15_REG1_VEPOS 24  // enable vectored interrupts (custom interrupt vectors)

#define CP15_REG1_SBO_MASK ((7<<4)|(1<<16)|(1<<18))
#define CP15_REG1_SBZ_MASK ((1<<17)|(3<<19)|(0x3F<<25))

#define CP15_REG2_Hi_ASIDPOS    5       // ASID field start
#define CP15_REG2_Hi_VPNPOS     12      // VPN field start
#define CP15_REG2_Hi_ASIDMASK   0x7F    // ASID extraction mask
#define CP15_REG2_Hi_VPNMASK    0x3FFFF // VPN extraction mask

#define CP15_REG8_IPOS      8   // next index to be written
#define CP15_REG10_IPOS     8   // next index to be written/searched
#define CP15_REG10_PFNPOS   13  // retrieved physical frame number
#define CP15_REG10_PPOS     31  // if P bit is 1 no match was found

#define CP15_REGTLB_IMASK   0x3F    // maks to extract index from Random and Index Registers


#endif //UARM_REGISTERS_H
