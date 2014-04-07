#ifndef UARM_LIBUARM_H
#define UARM_LIBUARM_H

/* prints a string *s 0-termianted on terminal 0 */
void tprint(char *s);

/* prints HALT message and terminates execution */
void HALT();

/* prints PANIC message and terminates execution */
void PANIC();

/* loads processor state stored at address *addr */
void LDST(void *addr);

/* stores current processor state at address *addr */
void STST(void *addr);

/* calls kernel system call handler passing sysNum in r0 */
void SYS(int sysNum);

#endif //UARM_LIBURAM_H
