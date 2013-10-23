.globl _start
_start:

BOOT:
    MOV r1, PC			/* populates exception vector */
    SUB r1, r1, #8		/* _start address */
    MOV r0, #0			/* excv vec pointer */
    MOV r3, #0xEA000000		/* branch preamble */

    ORR r2, r3, r1, LSR #2	/* builds the branch instr */
    STR r2, [r0], #4		/* stores instr in excv vec */

    ADD r2, r1, #UNDEF_H
    SUB r2, r2, r0
    ORR r2, r3, r2, LSR #2
    STR r2, [r0], #4

    ADD r2, r1, #SWI_H
    SUB r2, r2, r0
    ORR r2, r3, r2, LSR #2
    STR r2, [r0], #4

    ADD r2, r1, #PREFABT_H
    SUB r2, r2, r0
    ORR r2, r3, r2, LSR #2
    STR r2, [r0], #4

    ADD r2, r1, #DATAABT_H
    SUB r2, r2, r0
    ORR r2, r3, r2, LSR #2
    STR r2, [r0], #8

    ADD r2, r1, #IRQ_H
    SUB r2, r2, r0
    ORR r2, r3, r2, LSR #2
    STR r2, [r0], #4

    ADD r2, r1, #FIQ_H
    SUB r2, r2, r0
    ORR r2, r3, r2, LSR #2
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
    MOV sp, #0x10
    MSR SPSR_cf, sp		/* sets the spsr to user mode, thumb disabled and all interrupts enabled */
    MOV sp, #0xBF
    MOVS pc, lr

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
