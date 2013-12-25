#include "/usr/include/uarm/libuarm.h"

char *saluto = "Hello World!\n\0";
char *strano = "If this got printed something fishy has happened..\n\0";

int main(){

    tprint(saluto);

    HALT();

    tprint(strano);

    return 0;
}
