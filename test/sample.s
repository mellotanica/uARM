.globl _start
_start:

    MOV		r0, #0x2D0
    ADD		r0, r0, #4
    LDR		sp, [r0, #0]
    SUB		sp, sp, #0x1000
    MOV		r3, #3
    MOVs	r2, #3
    SUBNES	r1, r3, r2
    ADDEQ	r1, r1, #0xF0000000
    MOV		r2, #0xFF000000
    ADDs	r3, r1, r2
    CMPVS	r3, r1
    CMPCS	r2, r1
    STR		r3, [sp, #4]
    MOV		r3, #0
    LDR		r4, [sp, #-4]!
    LDR		r5, [sp, #1]!
    MOV		r1, #0xFA000000
    ORR		r3, r1, #0x00FB0000
    ORR		r1, r3, #0x0000FC00
    ORR		r3, r1, #0x000000FD
    STRH	r3, [sp, #-1]!
    LDRH	r1, [sp, #0]!
    LDRSH	r2, [sp, #0]
    LDRSB	r4, [sp, #0]
    MOV		r1, #0x000000FF
    ORR		r3, r1, #0x00000F00
    STRH	r3, [sp, #0]!
    LDRSH	r1, [sp, #0]
    LDRSB	r2, [sp, #0]
    MOV		r4, #3
LOOP_B:
    MOV	 r3, #3
LOOP_A:
    SUBS	r2, r3, #1
    MOV		r3, r2
    BLCC	LOOP_A
    SUB	    	r1, r4, #1
    MOV		r4, r1
    CMP		r4, #2
    BEQ		LOOP_B
    CMP		r4, #1
    SUB		r5, PC, #44
    BXEQ	r5
    ADD		r5, PC, #4
    BXAL	r5
    MOV		r1, #40
    MOV		r1, #4
