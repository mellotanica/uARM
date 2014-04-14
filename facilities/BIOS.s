.include "bios_const.h"

.equ ROMSTACK_TOP, 0x8000
.equ ROMSTACK_OFF, -0x10
.equ EXCV_BASE, 0x7000
.equ EXCV_INT_OLD, 0x0		/* Interrupt Old */
.equ EXCV_INT_NEW, 0x44		/* Interrupt New */
.equ EXCV_PGMT_OLD, 0x110	/* Program Trap Old */
.equ EXCV_PGMT_NEW, 0x154	/* Program Trap New */
.equ EXCV_SWI_OLD, 0x198	/* Syscall Old */
.equ EXCV_SWI_NEW, 0x1DC	/* Syscall New */
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
    ADD r2, r2, #60 /* skip to r15 */
    STR r8, [r2], #4
    MOV r9, #0x12
    STR r9, [r2]

    ADD r2, r0, #EXCV_PGMT_NEW
    ADD r2, r2, #60
    STR r8, [r2], #4
    MOV r9, #0x17
    STR r9, [r2]

    ADD r2, r0, #EXCV_SWI_NEW
    ADD r2, r2, #60
    STR r8, [r2], #4
    MOV r9, #0x13
    STR r9, [r2]

    MOV r3, #0x8000	/*retrieve entry point*/
    ADD r3, r3, #4
    LDR r2, [r3]
    MOV lr, r2		/* sets lr to entry point */
    MOV r0, #0x1F
    MSR SPSR_cf, r0	/* sets the spsr to system mode, thumb disabled and all interrupts enabled */
    ADD r0, r1, #HALT	/* exit point */
    MOV r2, #0x2D4	/* ramtop addr */
    LDR r3, [r2]
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
    STR lr, [sp], #4
    MRS lr, CPSR
    STR lr, [sp], #4
    STR r0, [sp], #4

    MRS lr, SPSR
    AND lr, lr, #0x1F
    CMP lr, #0x10
    MRS lr, SPSR
    ADDeq lr, lr, #0xF	/* if previus mode was user mode, switch to sys to backup registers */
    MSR CPSR, lr

    MOV r0, #EXCV_BASE	/* store registers */
    ADD r0, r0, #EXCV_SWI_OLD
    ADD r0, #4
    STMIA r0, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14}
    MOV sp, #ROMSTACK_TOP
    ADD sp, sp, #ROMSTACK_OFF
    ADD sp, sp, #8
    LDR r5, [sp], #-4	/* recover r0 from stack and store it in its slot */
    STR r5, [r0, #-4]!
    LDR r5, [sp], #-4	/* recover CPRS from stack and enable it */
    MSR CPSR, r5
    MRS r5, SPSR    /* store SPSR in state_old CPSR slot */
    ADD r0, r0, #PSR_OFFSET
    STR r5, [r0], #-4
    LDR r5, [sp, #-12]!	/* recover lr from stack and store it in old state pc slot */
    STR r5, [r0]

    LDR r6, [r5, #-4]	/* get SWI instruction */
    AND r6, r6, #0xFFFFFF
    CMP r6, #BIOS_SRV_SYS    /* if syscall requested load kernel defined handler */
    Bne SWI_H_Cont

    MOV ip, #EXCV_BASE
    ADD ip, ip, #EXCV_SWI_NEW
    ADD ip, ip, #PSR_OFFSET	/* put psr slot in new state, update status register */
    LDR lr, [ip]
    MSR CPSR, lr
    LDMIA ip, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15}

SWI_H_Cont:
    CMP r6, #BIOS_SRV_HALT
    Beq HALT

    CMP r6, #BIOS_SRV_PANIC
    Beq PANIC

    CMP r6, #BIOS_SRV_LDST
    Beq LDST

    B UNKNOWN_SRV

UNDEF_H:
DATAABT_H:
PREFABT_H:
    MOV sp, #ROMSTACK_TOP	/* save lr, CPSR and r0 onto stack */
    ADD sp, sp, #ROMSTACK_OFF
    STR lr, [sp], #4
    MRS lr, CPSR
    STR lr, [sp], #4
    STR r0, [sp], #4

    MRS lr, SPSR
    AND lr, lr, #0x1F
    CMP lr, #0x10
    MRS lr, SPSR
    ADDeq lr, lr, #0xF	/* if previus mode was user mode, switch to sys to backup registers */
    MSR CPSR, lr

    MOV r0, #EXCV_BASE	/* store registers */
    ADD r0, r0, #EXCV_SWI_OLD
    ADD r0, #4
    STMIA r0, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14}
    LDR r5, [sp, #-4]!	/* recover r0 from stack and store it in its slot */
    SUB r0, #4
    STR r5, [r0]
    LDR r5, [sp, #-4]!	/* recover CPRS from stack and enable it */
    MSR CPSR, r5
    MRS r5, SPSR    /* store SPSR in state_old CPSR slot */
    ADD r0, r0, #PSR_OFFSET
    STR r5, [r0], #-4
    LDR r5, [sp, #-4]!	/* recover lr from stack and store it in old state pc slot */
    STR r5, [r0]

    MOV ip, #EXCV_BASE
    ADD ip, ip, #EXCV_PGMT_NEW
    ADD sp, ip, #PSR_OFFSET	/* psr slot in new state, update status register */
    LDR lr, [sp]
    MSR CPSR, lr
    LDMIA ip, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15}

IRQ_H:
FIQ_H:
    MOV sp, #ROMSTACK_TOP	/* save lr, CPSR and r0 onto stack */
    ADD sp, sp, #ROMSTACK_OFF
    STR lr, [sp], #4
    MRS lr, CPSR
    STR lr, [sp], #4
    STR r0, [sp], #4

    MRS lr, SPSR
    AND lr, lr, #0x1F
    CMP lr, #0x10
    MRS lr, SPSR
    ADDeq lr, lr, #0xF	/* if previus mode was user mode, switch to sys to backup registers */
    MSR CPSR, lr

    MOV r0, #EXCV_BASE	/* store registers */
    ADD r0, r0, #EXCV_SWI_OLD
    ADD r0, #4
    STMIA r0, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14}
    LDR r5, [sp, #-4]!	/* recover r0 from stack and store it in its slot */
    SUB r0, #4
    STR r5, [r0]
    LDR r5, [sp, #-4]!	/* recover CPRS from stack and enable it */
    MSR CPSR, r5
    MRS r5, SPSR    /* store SPSR in state_old CPSR slot */
    ADD r0, r0, #PSR_OFFSET
    STR r5, [r0], #-4
    LDR r5, [sp, #-4]!	/* recover lr from stack and store it in old state pc slot */
    STR r5, [r0]

    MOV ip, #EXCV_BASE
    ADD ip, ip, #EXCV_INT_NEW
    ADD sp, ip, #PSR_OFFSET	/* psr slot in new state, update status register */
    LDR lr, [sp]
    MSR CPSR, lr
    LDMIA ip, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15}

/* Loads a processor state from given address *
 * unsigned int LDST(void *addr);             */
LDST:
    MOV ip, #EXCV_BASE
    ADD ip, ip, #EXCV_SWI_OLD
    LDR r0, [ip]
    ADD ip, ip, #PSR_OFFSET
    LDR r5, [ip]
    MSR CPSR, r5
    LDMIA r0, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15}

HALT:
    MOV r5, pc
    SUB r5, r5, #8  /* r5 = HALT */
    SUB r5, r5, #HALT /* r5 = _start */
    ADD r0, r5, #haltMess
    B PRINT

PANIC:
    MOV r5, pc
    SUB r5, r5, #8  /* r5 = PANIC */
    SUB r5, r5, #PANIC /* r5 = _start */
    ADD r0, r5, #panicMess
    B PRINT

UNKNOWN_SRV:
    MOV r5, pc
    SUB r5, r5, #8  /* r5 = UNKNOWN_SRV */
    SUB r5, r5, #UNKNOWN_SRV /* r5 = _start */
    ADD r0, r5, #unknownMess
    B PRINT

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
    Beq HALT_LOOP

    MOV r7, r7, LSL #8	/* setup char */
    ORR r7, r7, #2	/* add print command */
    STR r7, [r5]
    ADD r8, r8, #1
    B PRINT_LOOP

HALT_LOOP:
    B HALT_LOOP

haltMess:
    .asciz "SYSTEM HALTED.\0"

unknownMess:
    .asciz "UNKNOWN SERVICE.\nKERNEL PANIC!\0"

panicMess:
    .asciz "KERNEL PANIC!\0"
