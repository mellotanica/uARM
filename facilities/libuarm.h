#ifndef UARM_LIBUARM_H
#define UARM_LIBUARM_H

/* prints a string *s 0-termianted on terminal 0 */
void tprint(char *s);

/* prints HALT message and terminates execution */
void HALT();

/* prints PANIC message and terminates execution */
void PANIC();

//unsigned int LDST(void *addr);

#endif //UARM_LIBURAM_H
