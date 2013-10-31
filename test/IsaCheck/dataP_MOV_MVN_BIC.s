.global _start
_start:

    /* MOVE */

    MOV r0, #1
    MOV r1, #2
    MOV r2, #3
    MOV r3, #4
    MOV r4, #5
    MOV r5, #6
    MOV r6, #7
    MOV r7, #8
    MOV r8, #9
    MOV r9, #10
    MOV r10, #11
    MOV r11, #12
    MOV r12, #13
    MOV r13, #14
    MOV r14, #15

    MOVS r0, #0
    MOVS r1, #0
    MOVS r2, #0
    MOVS r3, #0
    MOVS r4, #0
    MOVS r5, #0
    MOVS r6, #0
    MOVS r7, #0
    MOVS r8, #0
    MOVS r9, #0
    MOVS r10, #0
    MOVS r11, #0
    MOVS r12, #0
    MOVS r13, #0
    MOVS r14, #0

    /* MOVE NOT and Shifts */

    MOV r0, #0xFA00
    MVN r1, r0
    MVN r2, #0xFA00
    MOV r3, r1, LSL #4
    MOV r4, r3, LSR #4
    MVN r5, r4
    MOV r6, r1, ASL #6
    MVN r7, r6, ASR #6
    MOV r8, r1, ROR #16
    MVN r9, r8, ROR #16

    MOV r0, #0
    MOV r1, #0
    MOV r2, #0
    MOV r3, #0
    MOV r4, #0
    MOV r5, #0
    MOV r6, #0
    MOV r7, #0
    MOV r8, #0
    MOV r9, #0

    /* BIC */

    MVN r0, #0
    BIC r1, r0, #1
    BIC r2, r0, #2
    BIC r3, r0, #4
    BIC r4, r0, #8
    BIC r5, r0, #0x10
    BIC r6, r0, #0x20
    BIC r7, r0, #0x40
    BIC r8, r0, #0x80
    BIC r1, r0, #0x100
    BIC r2, r0, #0x200
    BIC r3, r0, #0x400
    BIC r4, r0, #0x800
    BIC r5, r0, #0x1000
    BIC r6, r0, #0x2000
    BIC r7, r0, #0x4000
    BIC r8, r0, #0x8000
    BIC r1, r0, #0x10000
    BIC r2, r0, #0x20000
    BIC r3, r0, #0x40000
    BIC r4, r0, #0x80000
    BIC r5, r0, #0x100000
    BIC r6, r0, #0x200000
    BIC r7, r0, #0x400000
    BIC r8, r0, #0x800000
    BIC r1, r0, #0x1000000
    BIC r2, r0, #0x2000000
    BIC r3, r0, #0x4000000
    BIC r4, r0, #0x8000000
    BIC r5, r0, #0x10000000
    BIC r6, r0, #0x20000000
    BIC r7, r0, #0x40000000
    BIC r8, r0, #0x80000000

    MOV r11, #0x55000000
    ORR r11, r11, #0x00550000
    ORR r11, r11, #0x00005500
    ORR r11, r11, #0x00000055

    MOV r12, #0xAA000000
    ORR r12, r12, #0x00AA0000
    ORR r12, r12, #0x0000AA00
    ORR r12, r12, #0x000000AA

    BICS r9, r0, r11
    BICS r10, r0, r12


    MOV r0, #0x41000000
    ORR r0, r0, #0x00540000
    ORR r0, r0, #0x00004D00
    ORR r0, r0, #0x00000022

    MOV r1, #0x41000000
    BICS r2, r0, r1	/* res: 0x00544D22 */
    MOV r1, r1, LSR #16
    BICS r3, r0, r1	/* res: 0x41540C22 */
    MOV r1, r1, LSL #8
    BICS r4, r0, r1	/* res: 0x41144D22 */
    MOV r1, r1, LSR #16
    BICS r5, r0, r1	/* res: 0x41544D22 (unchanged) */

