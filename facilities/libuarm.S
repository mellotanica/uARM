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

/* prints a 0-terminated string on terminal 0 *
 * void tprint(const char *s)                 */

.global tprint

tprint:
    push {r5, r6, r7}
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
    pop {r5, r6, r7}
    BX lr

/* HALT and PANIC functions */

.global HALT

HALT:
HALT_start:
    MOV r1, pc
    SUB r1, r1, #8  /* r0 = HALT */
    SUB r1, r1, #HALT_start /* r0 = BOF */
    ADD r0, r1, #haltMess
    ADD lr, r1, #HALT_LOOP
    B tprint

.global PANIC

PANIC:
PANIC_start:
    MOV r1, pc
    SUB r1, r1, #8  /* r0 = PANIC */
    SUB r1, r1, #PANIC_start /* r0 = BOF */
    ADD r0, r1, #panicMess
    ADD lr, r1, #HALT_LOOP
    B tprint

HALT_LOOP:
    B HALT_LOOP

haltMess:
    .asciz "SYSTEM HALTED.\0"

panicMess:
    .asciz "KERNEL PANIC!\0"


/* Loads a processor state from given address *
 * unsigned int LDST(void *addr);             */

/*LDST:*/
