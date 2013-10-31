.global _start
_start:

    /* ADC */

    MOV r0, #0
    ADCS r0, r0, #0x000000FF
    ADCS r0, r0, #0x0000FF00
    ADCS r0, r0, #0x00FF0000
    ADCS r0, r0, #0xFF000000

    MOV r1, #0xAA
    ADCS r1, r1, #0x55
    ADCS r2, r1, r0

    ADCS r3, r0, #1
    ADCS r4, r0, r0

    MOV r0, #0		    /* r0 lo */
    ADD r0, r0, #0x000000FF
    ADD r0, r0, #0x0000FF00
    ADD r0, r0, #0x00FF0000
    ADD r0, r0, #0xFF000000

    MOV r1, #0x000000AC	    /* r0 hi */
    ADD r1, r1, #0x00003C00

    MOV r2, #0xFC000000	    /* r2 lo */
    ADD r2, r2, #0x00CA0000
    ADD r2, r2, #0x00001000
    ADD r2, r2, #0x00000033

    MOV r3, #0x00000021	    /* r2 hi */
    ADD r3, r3, #0x0000FF00
    ADD r3, r3, #0x05000000

    ADDS r4, r0, r2	    /* rSum lo: 0xFCCA1032 */
    ADCS r5, r1, r3	    /* rSum hi: 0x05013BCE */

    /* SBC */

    SBCS r1, r0, r0
    ADD r1, r1, #0xFF
    SBCS r2, r0, r1
    SBCS r3, r0, r1, LSL #8
    SBCS r4, r0, r1, LSL #16
    SBCS r5, r0, r1, LSL #24
    SBCS r6, r0, r1, LSL #10
    SBCS r7, r0, r1, LSL #17

    SBCS r8, r1, r0

    MOV r6, #0
    MOV r7, #0
    MOV r8, #0

    MOV r0, #0		    /* r0 lo */
    ADD r0, r0, #0x000000FF
    ADD r0, r0, #0x0000FF00
    ADD r0, r0, #0x00FF0000
    ADD r0, r0, #0xFF000000

    MOV r1, #0x000000AC	    /* r0 hi */
    ADD r1, r1, #0x00003C00

    MOV r2, #0xFC000000	    /* r2 lo */
    ADD r2, r2, #0x00CA0000
    ADD r2, r2, #0x00001000
    ADD r2, r2, #0x00000033

    MOV r3, #0x00000021	    /* r2 hi */
    ADD r3, r3, #0x0000FF00
    ADD r3, r3, #0x05000000

    SUBS r4, r0, r2	    /* rSub lo: 0x0335EFCC */
    SBCS r5, r1, r3	    /* rSub hi: 0xFAFF3D8B */

    /* RSC */

    RSCS r2, r1, r0
    MOV r3, r1, LSL #8
    RSCS r3, r3, r0
    MOV r4, r1, LSL #16
    RSCS r4, r4, r0
    MOV r5, r1, LSL #24
    RSCS r5, r5, r0
    MOV r6, r1, LSL #10
    RSCS r6, r6, r0
    MOV r7, r1, LSL #17
    RSCS r7, r7, r0

    RSCS r8, r0, r1

    MOV r6, #0
    MOV r7, #0
    MOV r8, #0

    MOV r0, #0		    /* r0 lo */
    ADD r0, r0, #0x000000FF
    ADD r0, r0, #0x0000FF00
    ADD r0, r0, #0x00FF0000
    ADD r0, r0, #0xFF000000

    MOV r1, #0x000000AC	    /* r0 hi */
    ADD r1, r1, #0x00003C00

    MOV r2, #0xFC000000	    /* r2 lo */
    ADD r2, r2, #0x00CA0000
    ADD r2, r2, #0x00001000
    ADD r2, r2, #0x00000033

    MOV r3, #0x00000021	    /* r2 hi */
    ADD r3, r3, #0x0000FF00
    ADD r3, r3, #0x05000000

    RSBS r4, r0, r2	    /* rSub lo: 0xFCCA1034 */
    RSCS r5, r1, r3	    /* rSub hi: 0x0500C274 */
