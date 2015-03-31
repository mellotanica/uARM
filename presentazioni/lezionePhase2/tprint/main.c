#include"iprint.h"
#include<libuarm.h>
#include<uARMconst.h>

char *msg = "TESTO DI PROVA";

void main(){
	unsigned int cpsr;
	tprint(msg);

	init();

	cpsr = getSTATUS();
	setSTATUS(STATUS_ALL_INT_ENABLE(cpsr));

	iprint(msg);
}
