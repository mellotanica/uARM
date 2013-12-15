.equ TERMCOMMAND,	0xC
.equ TERMSTATUS,	8
.equ SENDCHAR,		2
.equ READY_STAT,	1
.equ BUSY_STAT,		3
.equ TRANSMITTED_STAT,	5
.equ DEVBASE,		0x40
.equ DEVREGSIZE,	0x10
.equ DEVS_PER_IL,	8
.equ DEV_TERM_OFF,	4

.global tprint

/* prints a 0-terminated string on terminal 0 *
 * void tprint(const char *s)                 */
tprint:
    /*EOR r4, r4, r4	    /* r4 will hold the string index */
    MOV r5, #DEVREGSIZE
    MOV r6, #DEVS_PER_IL
    MUL r5, r6, r5
    MOV r6, #DEV_TERM_OFF
    MUL r5, r6, r5
    ADD r5, r5, #DEVBASE
    ADD r6, r5, #TERMSTATUS  /* terminal status register */
    ADD r5, r5, #TERMCOMMAND /* terminal command register */

tprint_print_loop:
    LDR r7, [r6]
    AND r7, r7, #0xFF
    CMP r7, #READY_STAT
    Beq tprint_do_print
    CMP r7, #TRANSMITTED_STAT
    Beq tprint_do_print
    CMP r7, #BUSY_STAT
    Beq tprint_print_loop
    B tprint_exit

tprint_do_print:
    LDRB r7, [r0], #1
    CMP r7, #0
    Beq tprint_exit

    MOV r7, r7, LSL #8
    ORR r7, r7, #SENDCHAR
    STR r7, [r5]
    B tprint_print_loop

tprint_exit:
    BX lr

/* Loads a processor state from given address *
 * unsigned int LDST(void *addr);             */

LDST:
