.global _start
_start:

    /* ORR */

    MOV r0, #0x00000055
    ORR r0, r0, #0x000000AA
    ORR r0, r0, #0x0000FF00

    MOV r1, #0xFF000000
    ORR r1, r1, #0x0000FF00

    ORRS r2, r1, r0
    ORRS r3, r0, r1
    ORRS r4, r0, r1, LSL #4
    ORRS r5, r0, r1, ASR #8

    ORRS r0, r0, r1
    ORRS r0, r0, #0x00FF0000

    /* AND */

    AND r2, r0, #0x00FF0000

    MOV r1, #0xFF000000
    ORR r1, r1, #0x00FE0000
    ORR r1, r1, #0x0000CC00
    ORR r1, r1, #0x000000AE

    ANDS r3, r0, r1

    BIC r0, r0, #0x00180000
    BIC r0, r0, #0x000005D0 /* r0: 0xFFE7FA2F */

    ANDS r4, r0, r1	    /* r4: 0xFFE6C82E */

    ANDS r5, r1, r1

    /* EOR */

    ORR r0, r0, #0xFF000000
    ORR r0, r0, #0x00FF0000
    ORR r0, r0, #0x0000FF00
    ORR r0, r0, #0x000000FF

    EORS r1, r0, #0xAA000000
    EORS r1, r1, #0x00AA0000
    EORS r1, r1, #0x0000AA00
    EORS r1, r1, #0x000000AA

    EORS r2, r0, r1
    EORS r3, r1, r0

    MOV r0, #0xFE000000
    ORR r0, r0, #0x00070000
    ORR r0, r0, #0x0000C500
    ORR r0, r0, #0x00000093

    MOV r1, #0x01000000
    ORR r1, r1, #0x00F80000
    ORR r1, r1, #0x00003A00
    ORR r1, r1, #0x0000006C

    EORS r2, r0, r1
    EORS r3, r1, r1
