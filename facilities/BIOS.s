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
    MOV r1, PC			/* populates exception vector */
    SUB r1, r1, #0x8		/* _start address */
    MOV r0, #0			/* excv vec pointer */
    MOV r3, #0xEA000000		/* branch preamble */

    MOV r2, r1, LSR #2
    ORR r2, r3, r2		/* builds the branch instr */
    STR r2, [r0], #4		/* stores instr in excv vec */

    ADD r2, r1, #UNDEF_H
    SUB r2, r2, r0
    MOV r2, r2, LSR #2
    ORR r2, r3, r2
    STR r2, [r0], #4

    ADD r2, r1, #SWI_H
    SUB r2, r2, r0
    MOV r2, r2, LSR #2
    ORR r2, r3, r2
    STR r2, [r0], #4

    ADD r2, r1, #PREFABT_H
    SUB r2, r2, r0
    MOV r2, r2, LSR #2
    ORR r2, r3, r2
    STR r2, [r0], #4

    ADD r2, r1, #DATAABT_H
    SUB r2, r2, r0
    MOV r2, r2, LSR #2
    ORR r2, r3, r2
    STR r2, [r0], #8

    ADD r2, r1, #IRQ_H
    SUB r2, r2, r0
    MOV r2, r2, LSR #2
    ORR r2, r3, r2
    STR r2, [r0], #4

    ADD r2, r1, #FIQ_H
    SUB r2, r2, r0
    MOV r2, r2, LSR #2
    ORR r2, r3, r2
    STR r2, [r0]

    /*each exception points to halt function by default*/
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
    MOV ip, #ROMSTACK_TOP	/* save sp and lr onto stack */
    ADD ip, ip, #ROMSTACK_OFF
    STR sp, [ip], #4
    STR lr, [ip], #4
    MRS lr, SPSR
    STR lr, [ip], #4

    AND lr, lr, #0x1F
    CMP lr, #0x10	/* if precedent mode was user mode store user mode registers else switch to precedent mode and store registers*/
    MRSne sp, CPSR
    STRne sp, [ip]
    BICne sp, sp, #0x1F
    ADDne sp, sp, lr

    MSRne CPSR_c, sp
    MOV ip, #EXCV_BASE	/* conditional store */
    ADD ip, ip, #EXCV_SWI_OLD
    STMeqIA ip, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14}^
    STMneIA ip, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14}
    ADD sp, ip, #60	/* pc slot, return value is held in lr */
    MOV ip, #ROMSTACK_TOP
    ADD ip, ip, #ROMSTACK_OFF
    LDR lr, [ip, #4]!	/* recover lr value from stack and write it in pc slot*/
    STR lr, [sp], #4
    LDR lr, [ip, #4]!	/* get old psr and store it in its slot */
    STR lr, [sp]

    MOV ip, #EXCV_BASE
    ADD ip, ip, #EXCV_SWI_NEW
    ADD sp, ip, #64	/* psr slot in new state, update status register */
    LDR lr, [sp]
    MSR CPSR, lr
    LDMIA ip, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15}

UNDEF_H:
DATAABT_H:
PREFABT_H:
    MOV ip, #ROMSTACK_TOP	/* save sp and lr onto stack */
    ADD ip, ip, #ROMSTACK_OFF
    STR sp, [ip], #4
    STR lr, [ip], #4
    MRS lr, SPSR
    STR lr, [ip], #4

    AND lr, lr, #0x1F
    CMP lr, #0x10	/* if precedent mode was user mode store user mode registers else switch to precedent mode and store registers*/
    MRSne sp, CPSR
    STRne sp, [ip]
    BICne sp, sp, #0x1F
    ADDne sp, sp, lr

    MSRne CPSR_c, sp
    MOV ip, #EXCV_BASE	/* conditional store */
    ADD ip, ip, #EXCV_PGMT_OLD
    STMeqIA ip, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14}^
    STMneIA ip, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14}
    ADD sp, ip, #60	/* pc slot, return value is held in lr */
    MOV ip, #ROMSTACK_TOP
    ADD ip, ip, #ROMSTACK_OFF
    LDR lr, [ip, #4]!	/* recover lr value from stack and write it in pc slot*/
    STR lr, [sp], #4
    LDR lr, [ip, #4]!	/* get old psr and store it in its slot */
    STR lr, [sp]

    MOV ip, #EXCV_BASE
    ADD ip, ip, #EXCV_PGMT_NEW
    ADD sp, ip, #64	/* psr slot in new state, update status register */
    LDR lr, [sp]
    MSR CPSR, lr
    LDMIA ip, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15}

IRQ_H:
FIQ_H:
    MOV ip, #ROMSTACK_TOP	/* save sp and lr onto stack */
    ADD ip, ip, #ROMSTACK_OFF
    STR sp, [ip], #4
    STR lr, [ip], #4
    MRS lr, SPSR
    STR lr, [ip], #4

    AND lr, lr, #0x1F
    CMP lr, #0x10	/* if precedent mode was user mode store user mode registers else switch to precedent mode and store registers*/
    MRSne sp, CPSR
    STRne sp, [ip]
    BICne sp, sp, #0x1F
    ADDne sp, sp, lr

    MSRne CPSR_c, sp
    MOV ip, #EXCV_BASE	/* conditional store */
    ADD ip, ip, #EXCV_INT_OLD
    STMeqIA ip, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14}^
    STMneIA ip, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14}
    ADD sp, ip, #60	/* pc slot, return value is held in lr */
    MOV ip, #ROMSTACK_TOP
    ADD ip, ip, #ROMSTACK_OFF
    LDR lr, [ip, #4]!	/* recover lr value from stack and write it in pc slot*/
    STR lr, [sp], #4
    LDR lr, [ip, #4]!	/* get old psr and store it in its slot */
    STR lr, [sp]

    MOV ip, #EXCV_BASE
    ADD ip, ip, #EXCV_INT_NEW
    ADD sp, ip, #64	/* psr slot in new state, update status register */
    LDR lr, [sp]
    MSR CPSR, lr
    LDMIA ip, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15}

HALT:
    MOV r0, pc
    SUB r0, r0, #8  /* r0 = HALT */
    SUB r0, r0, #HALT /* r0 = _start */
    ADD r5, r0, #haltMess
    B PRINT

PANIC:
    MOV r0, pc
    SUB r0, r0, #8  /* r0 = PANIC */
    SUB r0, r0, #PANIC /* r0 = _start */
    ADD r5, r0, #panicMess
    B PRINT

PRINT:
    MOV r1, #4
    MOV r2, #4
    MUL r1, r2, r1  /* dev reg size */
    MOV r2, #8
    MUL r1, r2, r1  /* devices per interrupt line */
    MOV r2, #4
    MUL r1, r2, r1  /* interrupt lines before terminal */
    ADD r1, r1, #DEV_BASE
    ADD r0, r1, #8	/* term0.TRANSM_STATUS */
    ADD r1, r1, #0xC	/* term0.TRANSM_COMMAND */
    MOV r4, #0	/* string counter */

PRINT_LOOP:
    LDR r3, [r0]
    AND r3, r3, #0xFF
    CMP r3, #3
    Beq PRINT_LOOP

    LDRB r3, [r5, r4]
    CMP r3, #0
    Beq HALT_LOOP

    MOV r3, r3, LSL #8	/* setup char */
    ORR r3, r3, #2	/* add print command */
    STR r3, [r1]
    ADD r4, r4, #1
    B PRINT_LOOP

HALT_LOOP:
    B HALT_LOOP

haltMess:
    .asciz "SYSTEM HALTED.\0"

panicMess:
    .asciz "KERNEL PANIC!\0"
