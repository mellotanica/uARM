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
}state_t; //to be replaced with actual processor state structure

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
void SYSCALL(unsigned int sysNum, unsigned int arg1, unsigned int arg2, unsigned int arg3);

/* access special registers */
unsigned int getSTATUS();
unsigned int getCAUSE();
unsigned int getTIMER();

/* set special registers value */
unsigned int setSTATUS(unsigned int status);
unsigned int setCAUSE(unsigned int cause);
unsigned int setTIMER(unsigned int timer);


#endif //UARM_LIBURAM_H
