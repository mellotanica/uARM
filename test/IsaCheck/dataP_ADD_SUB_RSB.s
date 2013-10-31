.global _start
_start:

    /* ADD */

    MOV r0, #0
    ADDS r0, r0, #0x000000FF
    ADDS r0, r0, #0x0000FF00
    ADDS r0, r0, #0x00FF0000
    ADDS r0, r0, #0xFF000000

    MOV r1, #0xAA
    ADDS r1, r1, #0x55
    ADDS r2, r1, r0

    ADDS r3, r0, #1
    ADDS r4, r0, r0

    /* SUB */

    SUBS r1, r0, r0
    ADD r1, r1, #0xFF
    SUBS r2, r0, r1
    SUBS r3, r0, r1, LSL #8
    SUBS r4, r0, r1, LSL #16
    SUBS r5, r0, r1, LSL #24
    SUBS r6, r0, r1, LSL #10
    SUBS r7, r0, r1, LSL #17

    SUBS r8, r1, r0

    /* RSB */

    RSBS r2, r1, r0
    MOV r3, r1, LSL #8
    RSBS r3, r3, r0
    MOV r4, r1, LSL #16
    RSBS r4, r4, r0
    MOV r5, r1, LSL #24
    RSBS r5, r5, r0
    MOV r6, r1, LSL #10
    RSBS r6, r6, r0
    MOV r7, r1, LSL #17
    RSBS r7, r7, r0

    RSBS r8, r0, r1
