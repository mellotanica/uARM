.equ ROMSTACK, 0x7FF0
.equ EXCV_INT_OLD 0x7000	/* Interrupt Old */
.equ EXCV_INT_NEW 0x7044	/* Interrupt New */
.equ EXCV_TLB_OLD 0x7088	/* TLB Old */
.equ EXCV_TLB_NEW 0x70CC	/* TLB New */
.equ EXCV_PGMT_OLD 0x7110	/* Program Trap Old */
.equ EXCV_PGMT_NEW 0x7154	/* Program Trap New */
.equ EXCV_SWI_OLD 0x7198	/* Syscall Old */
.equ EXCV_SWI_NEW 0x71DC	/* Syscall New */

.global _start
_start:

BOOT:
    MOV r1, PC			/* populates exception vector */
    SUB r1, r1, #0x10		/* _start address */
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
    STR r2, [r0, #0]

    MOV lr, #0x8000		/* sets lr to RAMBASE (hopefully main entry point) */
    MOV r0, #0x10
    MSR SPSR_cf, r0		/* sets the spsr to user mode, thumb disabled and all interrupts enabled */
    MOV r0, #0
    MOV r1, #0
    MOV r2, #0
    MOV r3, #0
    MOVS pc, lr			/* starts execution from ramBase in user mode */

UNDEF_H:
    MOV sp, #0x10
    MSR SPSR_cf, sp		/* sets the spsr to user mode, thumb disabled and all interrupts enabled */
    MOV sp, #0xAF
    MOVS pc, lr

SWI_H:
    /* store old state */
    ADR sp, EXCV_SWI_OLD
    STMIA sp!, {0,1,2,3,4,5,6,7}
    MOV r0, sp

    MRS SPSR_cf, r4
    AND r1, r4, #0x1F

    MRS CPSR_cf, r3
    AND r2, r3, #0xFFFFFFF0
    ORR r1, r1, r2
    MSR CPSR_cf, r2

    STMIA r0!, {8,9,10,11,12,13,14}
    MSR CPSR_cf, r3

    STR lr, [r0], #4
    STR r4, [r0, #0]

    /* recover interrupt cause */
    LDR r0, [lr, #-4]
    BIC r0, r0, #0xFF000000
    /* it should be moved to cp15 cause regiter.. */

    /* load new state */
    ADR sp, EXCV_SWI_NEW
    LSR r0, [sp, #0x40]
    MSR CPSR_cf, r0
    LDMIA sp, {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}

PREFABT_H:
    MOV sp, #0x10
    MSR SPSR_cf, sp		/* sets the spsr to user mode, thumb disabled and all interrupts enabled */
    MOV sp, #0xCF
    MOVS pc, lr

DATAABT_H:
    MOV sp, #0x10
    MSR SPSR_cf, sp		/* sets the spsr to user mode, thumb disabled and all interrupts enabled */
    MOV sp, #0xDF
    ADDS pc, lr, #4		/* temp.. skips to next instruction */

IRQ_H:
    MOV sp, #0x10
    MSR SPSR_cf, sp		/* sets the spsr to user mode, thumb disabled and all interrupts enabled */
    MOV sp, #0xEF
    MOVS pc, lr

FIQ_H:
    MOV sp, #0x10
    MSR SPSR_cf, sp		/* sets the spsr to user mode, thumb disabled and all interrupts enabled */
    MOV sp, #0xFF
    MOVS pc, lr
