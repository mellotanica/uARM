.include "bios_const.h"

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
.equ STATE_T_REGISTERS,	16

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

/* void SYSCALL(unsigned int sysNum, unsigned int arg1, unsigned int arg2, unsigned int arg3) */
.global SYS
SYS:
    SWI BIOS_SRV_SYS


/* HALT and PANIC functions */

.global HALT
HALT:
    SWI BIOS_SRV_HALT

.global PANIC
PANIC:
    SWI BIOS_SRV_PANIC

/* void LDST(void *addr) */
.global LDST
LDST:
    SWI BIOS_SRV_LDST

/* void STST(void *addr) */
.global STST
STST:
    STMIA r0, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15}

    push {r5,r6}

    ADD r5, r0, #PSR_OFFSET
    MRS CPSR, r6
    STR r6, [r5]

    pop {r5,r6}

    Bx lr

/* void WAIT() */
.global WAIT
WAIT:
    /* TODO: implement me */
    Bx lr


/* unsigned int getSTATUS() */
.global getSTATUS
getSTATUS:
    MRS CPSR, r0
    Bx lr

/* unsigned int getCAUSE() */
getCAUSE:
    /* TODO: implement me */
    Bx lr

/* unsigned int getTIMER() */
getTIMER:
    push {r5}
    MOV r5, #TIMER_ADDR
    LDR r0, [r5]
    pop {r5}
    Bx lr

/* unsigned int setSTATUS(unsigned int status) */
.global setSTATUS
setSTATUS:
    /* TODO: implement me */
    Bx lr

/* unsigned int setCAUSE(unsigned int cause) */
.global setCAUSE
setCAUSE:
    /* TODO: implement me */
    Bx lr

/* unsigned int setTIMER(unsigned int timer) */
.global setTIMER
setTIMER:
    /* TODO: implement me */
    Bx lr
