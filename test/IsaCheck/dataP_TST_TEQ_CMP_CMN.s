.macro ckcond
    MOV r11, #0
    MOV r12, #0
    ORREQ r11, #0x00000001  /* Equal */
    ORRNE r11, #0x00000010  /* not Equal */
    ORRCS r11, #0x00000100  /* Carry Set / Unsigned higher or same */
    ORRCC r11, #0x00001000  /* Carry Clear / Unsigned lower */
    ORRMI r11, #0x00010000  /* Negative */
    ORRPL r11, #0x00100000  /* Positive or Zero */
    ORRVS r11, #0x01000000  /* Overflow */
    ORRVC r11, #0x10000000  /* no Overflow */
    ORRHI r12, #0x00000001  /* Unsigned higher */
    ORRLS r12, #0x00000010  /* Unsigned lower or same */
    ORRGE r12, #0x00000100  /* Signed greater than or equal */
    ORRLT r12, #0x00001000  /* Signed less than */
    ORRGT r12, #0x00010000  /* Signed greater than */
    ORRLE r12, #0x00100000  /* Signed less than or equal */
.endm

.global _start
_start:

    MOV r0, #0
    MOV r1, #1
    MOV r2, #0x10
    MOV r3, #16
    SUB r4, r0, #1
    MOV r5, #0xFF000000
    ORR r5, r5, #0xFF0000
    ORR r5, r5, #0xFF00
    ORR r5, r5, #0xFF

    /* CMP */

    CMP r0, r0
    ckcond
    CMP r0, #0
    ckcond
    CMP r1, r0
    ckcond
    CMP r0, r1
    ckcond
    CMP r1, r1
    ckcond
    CMP r2, r3
    ckcond
    CMP r3, r4
    ckcond
    CMP r4, r5
    ckcond
    CMP r5, r5
    ckcond
    CMP r4, r0
    ckcond
    CMP r0, r5
    ckcond

    /* CMN */

    CMN r0, r0
    ckcond
    CMN r0, #0
    ckcond
    CMN r1, r0
    ckcond
    CMN r0, r1
    ckcond
    CMN r1, r1
    ckcond
    CMN r2, r3
    ckcond
    CMN r3, r4
    ckcond
    CMN r4, r5
    ckcond
    CMN r5, r5
    ckcond
    CMN r4, r0
    ckcond
    CMN r0, r5
    ckcond

    /* TST */

    TST r0, r0
    ckcond
    TST r0, #0
    ckcond
    TST r1, r0
    ckcond
    TST r0, r1
    ckcond
    TST r1, r1
    ckcond
    TST r2, r3
    ckcond
    TST r3, r4
    ckcond
    TST r4, r5
    ckcond
    TST r5, r5
    ckcond
    TST r4, r0
    ckcond
    TST r0, r5
    ckcond

    /* TEQ */

    TEQ r0, r0
    ckcond
    TEQ r0, #0
    ckcond
    TEQ r1, r0
    ckcond
    TEQ r0, r1
    ckcond
    TEQ r1, r1
    ckcond
    TEQ r2, r3
    ckcond
    TEQ r3, r4
    ckcond
    TEQ r4, r5
    ckcond
    TEQ r5, r5
    ckcond
    TEQ r4, r0
    ckcond
    TEQ r0, r5
    ckcond
