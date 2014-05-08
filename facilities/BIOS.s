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

.equ STATE_T_SIZE, 88
.equ ROMSTACK_TOP, 0x8000
.equ ROMSTACK_OFF, -0x10
.equ EXCV_BASE, 0x7000
.equ EXCV_INT_OLD, 0	/* Interrupt Old */
.equ EXCV_INT_NEW, STATE_T_SIZE	/* Interrupt New */
.equ EXCV_TLB_OLD, 2 * STATE_T_SIZE	/* TLB Exception Old */
.equ EXCV_TLB_NEW, 3 * STATE_T_SIZE	/* TLB Exception New */
.equ EXCV_PGMT_OLD, 4 * STATE_T_SIZE	/* Program Trap Old */
.equ EXCV_PGMT_NEW, 5 * STATE_T_SIZE	/* Program Trap New */
.equ EXCV_SWI_OLD, 6 * STATE_T_SIZE	/* Syscall Old */
.equ EXCV_SWI_NEW, 7 * STATE_T_SIZE	/* Syscall New */
.equ DEV_BASE, 0x40

.global _start
_start:

BOOT:
    MOV r1, pc			/* populates exception vector */
    SUB r1, r1, #0x8		/* _start address */
    MOV r0, #0			/* excv vec pointer */
    MOV r3, #0xEA000000		/* branch preamble */

    MOV r2, r1, LSR #2
    ORR r2, r3, r2		/* builds the branch instr */
    STR r2, [r0], #4		/* stores instr in excv vec */

    ADD r2, r1, #UNDEF_H	/* set r2 to handler address */
    SUB r2, r2, r0		/* subtract exception vector address from r2 */
    SUB r2, r2, #8		/* minus 2 words (ps is always 2 words forward) */
    MOV r2, r2, LSR #2		/* shift right to get actual immediate value */
    ORR r2, r3, r2		/* add branch preamble */
    STR r2, [r0], #4

    ADD r2, r1, #SWI_H
    SUB r2, r2, r0
    SUB r2, r2, #8
    MOV r2, r2, LSR #2
    ORR r2, r3, r2
    STR r2, [r0], #4

    ADD r2, r1, #PREFABT_H
    SUB r2, r2, r0
    SUB r2, r2, #8
    MOV r2, r2, LSR #2
    ORR r2, r3, r2
    STR r2, [r0], #4

    ADD r2, r1, #DATAABT_H
    SUB r2, r2, r0
    SUB r2, r2, #8
    MOV r2, r2, LSR #2
    ORR r2, r3, r2
    STR r2, [r0], #8

    ADD r2, r1, #IRQ_H
    SUB r2, r2, r0
    SUB r2, r2, #8
    MOV r2, r2, LSR #2
    ORR r2, r3, r2
    STR r2, [r0], #4

    ADD r2, r1, #FIQ_H
    SUB r2, r2, r0
    SUB r2, r2, #8
    MOV r2, r2, LSR #2
    ORR r2, r3, r2
    STR r2, [r0]

    /*each exception points to panic function by default*/
    MOV r0, #EXCV_BASE
    ADD r8, r1, #PANIC
    ADD r2, r0, #EXCV_INT_NEW
    ADD r2, r2, #PSR_OFFSET /* skip to r15 */
    SUB r2, r2, #4
    STR r8, [r2], #4
    MOV r9, #0xD2
    STR r9, [r2]

    ADD r2, r0, #EXCV_PGMT_NEW
    ADD r2, r2, #PSR_OFFSET /* skip to r15 */
    SUB r2, r2, #4
    STR r8, [r2], #4
    MOV r9, #0xDB
    STR r9, [r2]

    ADD r2, r0, #EXCV_SWI_NEW
    ADD r2, r2, #PSR_OFFSET /* skip to r15 */
    SUB r2, r2, #4
    STR r8, [r2], #4
    MOV r9, #0xD3
    STR r9, [r2]

    ADD r2, r0, #EXCV_TLB_NEW
    ADD r2, r2, #PSR_OFFSET /* skip to r15 */
    SUB r2, r2, #4
    STR r8, [r2], #4
    MOV r9, #0xD7
    STR r9, [r2]

    MOV r3, #0x8000	/*retrieve entry point*/
    ADD r3, r3, #4
    LDR r2, [r3]
    MOV lr, r2		/* sets lr to entry point */
    MOV r0, #0xDF
    MSR SPSR_cf, r0	/* sets the spsr to system mode, thumb disabled and all interrupts disabled */
    ADD r0, r1, #HALT	/* exit point */
    MOV r2, #RAMSIZE_INFO	/* ramtop addr */
    LDR r3, [r2]
    MOV r2, #RAMBASE_INFO
    LDR r1, [r2]
    ADD r3, r3, r1
    SUB r2, r3, #8
    STR r3, [r2]
    STR r0, [r2, #4]
    LDMIA r2, {sp, lr}^
    MOV r0, #0
    MOV r1, #0
    MOV r2, #0
    MOV r3, #0
    MOV r8, #0
    MOV r9, #0
    MOVS pc, lr			/* starts execution from ramBase in user mode */

SWI_H:
    MOV sp, #ROMSTACK_TOP	/* save lr, CPSR and r0 onto stack */
    ADD sp, sp, #ROMSTACK_OFF
    STR r0, [sp], #4
    STR lr, [sp], #4
    MRS r0, CPSR
    STR r0, [sp]

    MOV r0, #EXCV_BASE	/* store registers */
    ADD r0, r0, #EXCV_SWI_OLD

    BL SAVE_OLD_STATE

    MOV sp, #ROMSTACK_TOP
    ADD sp, sp, #ROMSTACK_OFF
    ADD sp, sp, #4
    LDR lr, [sp]

    LDR r6, [lr, #-4]	/* get SWI instruction */
    AND r6, r6, #0xFFFFFF
    CMP r6, #BIOS_SRV_SYS    /* if syscall requested load kernel defined handler */
    Bne SWI_H_Cont

    MOV r0, #EXCV_BASE
    ADD r0, r0, #EXCV_SWI_NEW
    B LDST

SWI_H_Cont:
    MOV sp, #EXCV_BASE
    ADD sp, sp, #EXCV_SWI_OLD
    LDM sp, {r0, r1, r2, r3}

    CMP r6, #BIOS_SRV_HALT
    Beq HALT

    CMP r6, #BIOS_SRV_PANIC
    Beq PANIC

    CMP r6, #BIOS_SRV_LDST
    Beq LDST

    CMP r6, #BIOS_SRV_WAIT
    Beq WAIT

    CMP r6, #BIOS_SRV_BP
    Beq UNKNOWN_SRV /* FIXME: implement BREAK*/

    B UNKNOWN_SRV

UNDEF_H:
    MOV sp, #ROMSTACK_TOP	/* save lr, CPSR and r0 onto stack */
    ADD sp, sp, #ROMSTACK_OFF
    STR r0, [sp], #4
    STR lr, [sp], #4
    MRS r0, CPSR
    STR r0, [sp]

    MOV r0, #EXCV_BASE	/* store registers */
    ADD r0, r0, #EXCV_PGMT_OLD

    BL SAVE_OLD_STATE

    MOV r0, #EXCV_BASE
    ADD r0, r0, #EXCV_PGMT_NEW
    B LDST

DATAABT_H:
PREFABT_H:
    MOV sp, #ROMSTACK_TOP	/* save lr, CPSR and r0 onto stack */
    ADD sp, sp, #ROMSTACK_OFF
    STR r0, [sp], #4
    STR lr, [sp], #4
    MRS r0, CPSR
    STR r0, [sp]

    MOV r0, #EXCV_BASE	/* store registers */
    ADD r0, r0, #EXCV_TLB_OLD

    BL SAVE_OLD_STATE

    MOV r0, #EXCV_BASE
    ADD r0, r0, #EXCV_TLB_NEW
    B LDST

IRQ_H:
FIQ_H:
    MOV sp, #ROMSTACK_TOP	/* save lr, CPSR and r0 onto stack */
    ADD sp, sp, #ROMSTACK_OFF
    STR r0, [sp], #4
    STR lr, [sp], #4
    MRS r0, CPSR
    STR r0, [sp]

    MOV r0, #EXCV_BASE	/* store registers */
    ADD r0, r0, #EXCV_INT_OLD

    BL SAVE_OLD_STATE

    MOV r0, #EXCV_BASE
    ADD r0, r0, #EXCV_INT_NEW
    B LDST

HALT:
    MOV r5, pc
    SUB r5, r5, #8  /* r5 = HALT */
    SUB r5, r5, #HALT /* r5 = _start */
    ADD r0, r5, #haltMess
    Bl PRINT
    CDP p15, 0xF, c0, c0, c1, 0
    NOP
    NOP

PANIC:
    MOV r5, pc
    SUB r5, r5, #8  /* r5 = PANIC */
    SUB r5, r5, #PANIC /* r5 = _start */
    ADD r0, r5, #panicMess
    Bl PRINT
    B HALT_LOOP
    NOP
    NOP

UNKNOWN_SRV:
    MOV r5, pc
    SUB r5, r5, #8  /* r5 = UNKNOWN_SRV */
    SUB r5, r5, #UNKNOWN_SRV /* r5 = _start */
    ADD r0, r5, #unknownMess
    Bl PRINT
    B HALT_LOOP

HALT_LOOP:
    B HALT_LOOP

PRINT:
    MOV r5, #4
    MOV r6, #4
    MUL r5, r6, r5  /* dev reg size */
    MOV r6, #8
    MUL r5, r6, r5  /* devices per interrupt line */
    MOV r6, #4
    MUL r5, r6, r5  /* interrupt lines before terminal */
    ADD r5, r5, #DEV_BASE
    ADD r4, r5, #8	/* term0.TRANSM_STATUS */
    ADD r5, r5, #0xC	/* term0.TRANSM_COMMAND */
    MOV r8, #0	/* string counter */

PRINT_LOOP:
    LDR r7, [r4]
    AND r7, r7, #0xFF
    CMP r7, #3
    Beq PRINT_LOOP

    LDRB r7, [r0, r8]
    CMP r7, #0
    BXeq lr

    MOV r7, r7, LSL #8	/* setup char */
    ORR r7, r7, #2	/* add print command */
    STR r7, [r5]
    ADD r8, r8, #1
    B PRINT_LOOP

/* Loads a processor state from given address *
 * unsigned int LDST(void *addr);             */
LDST:
    MOV ip, r0
    ADD ip, ip, #PSR_OFFSET
    LDR r5, [ip], #4    /* r5 contains current psr */

    LDR r7, [ip], #4	/* restore coprocessor registers */
    MCR p15, #0, r7, c1, c0, #0
    LDR r7, [ip], #4
    MCR p15, #0, r7, c2, c0, #0
    MCR p15, #0, r7, c2, c0, #1
    LDR r7, [ip]
    MCR p15, #0, r7, c15, c0
    MCR p15, #1, r7, c15, c0

    MSR CPSR, r5
    LDMIA r0, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15}

/* r0 contains state_old addr, lr points to caller and sp to stack with old lr, CPSR and r0 */
SAVE_OLD_STATE:
    MRS sp, SPSR

    AND sp, sp, #0xF
    CMP sp, #0
    MRS sp, SPSR
    ADDeq sp, sp, #0xF
    ORR sp, sp, #0xC0

    MSR CPSR, sp

    ADD r0, r0, #4
    STMIA r0, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14}
    SUB r0, r0, #4

    MOV sp, #ROMSTACK_TOP
    ADD sp, sp, #ROMSTACK_OFF

    LDR r1, [sp], #4	/* r0 */
    STR r1, [r0]

    LDR r2, [sp], #4	/* pc */
    ADD r0, r0, #PSR_OFFSET
    SUB r0, r0, #4
    STR r2, [r0], #4

    LDR r3, [sp]    /* cpsr */
    MSR CPSR, r3

    MRS r3, SPSR
    STR r3, [r0], #4

    MRC p15, #0, r6, c1, c0, #0	    /* CP15_Control */
    MRC p15, #0, r7, c2, c0	    /* CP15_EntryHi */
    MRC p15, #0, r8, c15, c0	    /* CP15_CAUSE */
    STMIA r0, {r6, r7, r8}

    MOV r6, #TOD_HI_INFO    /*save TOD*/
    LDR r7, [r6]
    MOV r6, #TOD_LO_INFO
    LDR r8, [r6]
    STMIA r0, {r7, r8}

    BX lr

    .asciz "paddingpadd"

haltMess:
    .asciz "SYSTEM HALTED.\0"

unknownMess:
    .asciz "UNKNOWN SERVICE.\nKERNEL PANIC!\0"

panicMess:
    .asciz "KERNEL PANIC!\0"

