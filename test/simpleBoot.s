.globl _start
_start:
	
MOV lr, #0x8000		/* sets lr to RAMBASE (hopefully main entry point) */
MOV r0, #0x10
MSR SPSR_cf, r0		/* sets the spsr to user mode, thumb disabled and all interrupts enabled */
MOV r0, #0
MOVS pc, lr			/* starts execution from ramBase in user mode */
