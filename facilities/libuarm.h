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
unsigned int getTLB_Index();
unsigned int getTLB_Random();
unsigned int getEntryHi();
unsigned int getEntryLo();
unsigned int getBadVAddr();

/* set special registers value */
unsigned int setSTATUS(unsigned int status);
unsigned int setCAUSE(unsigned int cause);
unsigned int setTIMER(unsigned int timer);
unsigned int setCONTROL(unsigned int control);
unsigned int setTLB_Index(unsigned int index);
unsigned int setEntryHi(unsigned int hi);
unsigned int setEntryLo(unsigned int lo);

/* coprocessor TLB calls */
void TLBWR();
void TLBWI();
void TLBR();
void TLBP();
void TLBCLR();

#endif //UARM_LIBURAM_H
