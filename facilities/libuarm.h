/*
 * uARM
 *
 * Copyright (C) 2014 Marco Melletti
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef UARM_LIBUARM_H
#define UARM_LIBUARM_H

typedef struct{
    unsigned int a1;    //r0
    unsigned int a2;    //r1
    unsigned int a3;    //r2
    unsigned int a4;    //r3
    unsigned int v1;    //r4
    unsigned int v2;    //r5
    unsigned int v3;    //r6
    unsigned int v4;    //r7
    unsigned int v5;    //r8
    unsigned int v6;    //r9
    unsigned int sl;    //r10
    unsigned int fp;    //r11
    unsigned int ip;    //r12
    unsigned int sp;    //r13
    unsigned int lr;    //r14
    unsigned int pc;    //r15
    unsigned int cpsr;
    unsigned int CP15_Control;
    unsigned int CP15_EntryHi;
    unsigned int CP15_Cause;
    unsigned int TOD_Hi;
    unsigned int TOD_Low;
}state_t;

/* prints a string *s 0-termianted on terminal 0 */
void tprint(char *s);

/* prints HALT message and terminates execution */
void HALT();

/* prints PANIC message and terminates execution */
void PANIC();

/* put the machine in idle state waiting for interrupts */
void WAIT();

/* loads processor state stored at address *addr */
void LDST(void *addr);

/* stores current processor state at address *addr */
void STST(void *addr);

/* call kernel system call handler */
int SYSCALL(unsigned int sysNum, unsigned int arg1, unsigned int arg2, unsigned int arg3);

/* rasie breakpoint exception */
void BREAK(unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3);

/* access special registers */
unsigned int getSTATUS();
unsigned int getCAUSE();
unsigned int getTIMER();
unsigned int getTODHI();
unsigned int getTODLO();
unsigned int getCONTROL();

/* set special registers value */
unsigned int setSTATUS(unsigned int status);
unsigned int setCAUSE(unsigned int cause);
unsigned int setTIMER(unsigned int timer);
unsigned int setCONTROL(unsigned int control);


#endif //UARM_LIBURAM_H
