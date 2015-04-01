#include<libuarm.h>
#include<arch.h>
#include<uARMconst.h>
#include<uARMtypes.h>

unsigned int status;
int int_line, dev_num;
int notify_interrupt;

void intCallback(){
	state_t *returnState = (state_t*) INT_OLDAREA;
	int i, *devVect;
	dtpreg_t *dev_g;
	termreg_t *dev_t;

	if(notify_interrupt){
		notify_interrupt = 0;
		tprint("intCallback: handling interrupt\n");
	}

	int_line = getCAUSE();
	for(i = 0; i < 8; i++){
		if(CAUSE_IP_GET(int_line, i)){
			int_line = i;
			break;
		}
	}
	devVect = (unsigned int*) 0x6FE0 + (i*4);
	dev_num = *devVect;
	for(i = 0; i < 8; i++){
		if(dev_num & (1 << i)){
			dev_num = i;
			break;
		}
	}
	if(int_line == IL_TERMINAL){
		dev_t = (termreg_t*) DEV_REG_ADDR(int_line, dev_num);
		status = dev_t->transm_status;
		dev_t->transm_command = DEV_C_ACK;
	} else {
		dev_g = (dtpreg_t*) DEV_REG_ADDR(int_line, dev_num);
		status = dev_g->status;
		dev_g->command = DEV_C_ACK;
	}

	returnState->pc -= 4;
	LDST(returnState);
}

void sysCallback(){
	state_t *returnState = (state_t*) SYSBK_OLDAREA;
	tprint("sysCallback: raised syscall!\n");
	LDST(returnState);
}

void iprint(char *msg){
	char *s = msg;
	termreg_t *dev = (termreg_t *) DEV_REG_ADDR(INT_TERMINAL, 0);

	while (*s != '\0') {
		/* Put "transmit char" command+char in term0 register (3rd word). This 
		 *                  *                    actually starts the operation on the device! */
		dev->transm_command = DEV_TTRS_C_TRSMCHAR | (((unsigned int) *s) << 8);

		/* Wait for I/O completion (SYS8) */
		WAIT();
		
		if ((status & DEV_TERM_STATUS) != DEV_TTRS_S_CHARTRSM)
			PANIC();

		if (((status >> 8) & DEV_TERM_STATUS) != *s)
			PANIC();

		s++;    
	}

}

int main(){
	state_t *statep;
	unsigned int *ramtop = (unsigned int *) BUS_REG_RAM_SIZE;

	tprint("main: setting up syscalls\n");
	statep = (state_t*) SYSBK_NEWAREA;
	STST(statep);
	statep->pc = (unsigned int) sysCallback;

	tprint("main: raising syscall\n");
	SYSCALL(0, 0, 0, 0);

	tprint("main: setting up interrutps\n");
	statep = (state_t*) INT_NEWAREA;
	STST(statep);
	statep->pc = (unsigned int) intCallback;
	statep->sp = *ramtop + RAM_BASE - PAGE_SIZE;
	notify_interrupt = 1;

	tprint("main: enabling interrutps\n");
	setSTATUS(STATUS_ALL_INT_ENABLE(getSTATUS()));

	tprint("main: try iprint\n");
	iprint("main: iprinted!\n");

	HALT();
	return 0;
}
