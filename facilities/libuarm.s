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

.include "bios_const.h"

.equ TERMCOMMAND,	0xC
.equ TERMSTATUS,	8
.equ SENDCHAR,		2
.equ READY_STAT,	1
.equ BUSY_STAT,		3
.equ TRANSMITTED_STAT,	5
.equ DEVBASE,		0x40
.equ DEVREGSIZE,	0x10
.equ DEVS_PER_IL,	8
.equ DEV_TERM_OFF,	4
.equ STATE_T_REGISTERS,	16

.equ OP_TLBWR, 2
.equ OP_TLBWI, 3
.equ OP_TLBR, 4
.equ OP_TLBP, 5
.equ OP_TLBCLR, 6


/* prints a 0-terminated string on terminal 0 *
 * void tprint(const char *s)                 */

.global tprint

tprint:
    push {r5, r6, r7}
    MOV r5, #DEVREGSIZE
    MOV r6, #DEVS_PER_IL
    MUL r5, r6, r5
    MOV r6, #DEV_TERM_OFF
    MUL r5, r6, r5
    ADD r5, r5, #DEVBASE
    ADD r6, r5, #TERMSTATUS  /* terminal status register */
    ADD r5, r5, #TERMCOMMAND /* terminal command register */

tprint_print_loop:
    LDR r7, [r6]
    AND r7, r7, #0xFF
    CMP r7, #READY_STAT
    Beq tprint_do_print
    CMP r7, #TRANSMITTED_STAT
    Beq tprint_do_print
    CMP r7, #BUSY_STAT
    Beq tprint_print_loop
    B tprint_exit

tprint_do_print:
    LDRB r7, [r0], #1
    CMP r7, #0
    Beq tprint_exit

    MOV r7, r7, LSL #8
    ORR r7, r7, #SENDCHAR
    STR r7, [r5]
    B tprint_print_loop

tprint_exit:
    pop {r5, r6, r7}
    BX lr

/* void SYSCALL(unsigned int sysNum, unsigned int arg1, unsigned int arg2, unsigned int arg3) */
.global SYSCALL
SYSCALL:
    SWI BIOS_SRV_SYS
    BX lr

.global BREAK
BREAK:
    SWI BIOS_SRV_BP
    BX lr

/* HALT and PANIC functions */
.global HALT
HALT:
    SWI BIOS_SRV_HALT

.global PANIC
PANIC:
    SWI BIOS_SRV_PANIC

/* void LDST(void *addr) */
.global LDST
LDST:
    SWI BIOS_SRV_LDST
    BX lr   /* probably useless... */

/* void STST(void *addr) */
.global STST
STST:
    STMIA r0, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15}

    push {r5, r6, r7, r8}

    ADD r5, r0, #PSR_OFFSET
    MRS r6, CPSR
    STR r6, [r5], #4

    MRC p15, #0, r6, c1, c0, #0	    /* CP15_Control */
    MRC p15, #0, r7, c2, c0	    /* CP15_EntryHi */
    MRC p15, #0, r8, c15, c0	    /* CP15_CAUSE */

    STMIA r5, {r6, r7, r8}

    MOV r6, #TOD_HI_INFO
    LDR r7, [r6]
    MOV r6, #TOD_LO_INFO
    LDR r8, [r6]

    STMIA r5, {r7, r8}

    pop {r5, r6, r7, r8}

    Bx lr

/* void WAIT() */
.global WAIT
WAIT:
    CDP p15, 0xF, c0, c0, c1, 1


/* unsigned int getSTATUS() */
.global getSTATUS
getSTATUS:
    MRS r0, CPSR
    Bx lr

/* unsigned int getCAUSE() */
.global getCAUSE
getCAUSE:
    MRC p15, #0, r0, c15, c0
    Bx lr

/* unsigned int getCONTROL() */
.global getCONTROL
getCONTROL:
    MRC p15, #0, r0, c1, c0, #0
    Bx lr

/* unsigned int getTLB_Index() */
.global getTLB_Index
getTLB_Index:
    MRC p15, #0, r0, c10, c0, #0
    Bx lr

/* unsigned int getTLB_Random() */
.global getTLB_Random
getTLB_Random:
    MRC p15, #0, r0, c8, c0, #0
    Bx lr

/* unsigned int getEntryHi() */
.global getEntryHi
getEntryHi:
    MRC p15, #0, r0, c2, c0, #0
    Bx lr

/* unsigned int getEntryLo() */
.global getEntryLo
getEntryLo:
    MRC p15, #1, r0, c2, c0, #0
    Bx lr

/* unsigned int getBadVAddr() */
.global getBadVAddr
getBadVAddr:
    MRC p15, #0, r0, c6, c0, #0
    Bx lr

/* unsigned int getTIMER() */
.global getTIMER
getTIMER:
    push {r5}
    MOV r5, #TIMER_INFO
    LDR r0, [r5]
    pop {r5}
    Bx lr

/* unsigned int getTODHI() */
.global getTODHI
getTODHI:
    push {r5}
    MOV r5, #TOD_HI_INFO
    LDR r0, [r5]
    pop {r5}
    Bx lr

/* unsigned int getTODLO() */
.global getTODLO
getTODLO:
    push {r5}
    MOV r5, #TOD_LO_INFO
    LDR r0, [r5]
    pop {r5}
    Bx lr

/* unsigned int setSTATUS(unsigned int status) */
.global setSTATUS
setSTATUS:
    MSR CPSR, r0
    Bx lr

/* unsigned int setCAUSE(unsigned int cause) */
.global setCAUSE
setCAUSE:
    MCR p15, #0, r0, c15, c0
    Bx lr

/* unsigned int setCONTROL(unsigned int cause) */
.global setCONTROL
setCONTROL:
    MCR p15, #0, r0, c1, c0, #0
    Bx lr

/* unsigned int setTLB_Index(unsigned int index) */
.global setTLB_Index
setTLB_Index:
    MCR p15, #0, r0, c10, c0, #0
    Bx lr

/* unsigned int setEntryHi(unsigned int hi) */
.global setEntryHi
setEntryHi:
    MCR p15, #0, r0, c2, c0, #0
    MCR p15, #0, r0, c2, c0, #1
    Bx lr

/* unsigned int setEntryLo(unsigned int lo) */
.global setEntryLo
setEntryLo:
    MCR p15, #1, r0, c2, c0, #0
    Bx lr

/* unsigned int setTIMER(unsigned int timer) */
.global setTIMER
setTIMER:
    push {r5}
    MOV r5, #TIMER_INFO
    STR r0, [r5]
    pop {r5}
    Bx lr

/* void TLBWR(); */
.global TLBWR
TLBWR:
    CDP p15, #OP_TLBWR, c0, c8, c0, #0
    NOP
    NOP
    Bx lr

/* void TLBWI(); */
.global TLBWI
TLBWI:
    CDP p15, #OP_TLBWI, c0, c10, c0, #0
    NOP
    NOP
    Bx lr

/* void TLBR(); */
.global TLBR
TLBR:
    CDP p15, #OP_TLBR, c2, c10, c0, #0
    Bx lr

/* void TLBP(); */
.global TLBP
TLBP:
    CDP p15, #OP_TLBP, c10, c2, c0, #0
    Bx lr

/* void TLBCLR(); */
.global TLBCLR
TLBCLR:
    CDP p15, #OP_TLBCLR, c0, c0, c0, #0
    NOP
    NOP
    Bx lr
