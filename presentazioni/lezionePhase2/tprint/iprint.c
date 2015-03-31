#include<arch.h>
#include<libuarm.h>
#include<uARMconst.h>
#include<uARMtypes.h>

#define SIGNAL 0xFFFFFFFF

void breakfunc() {}


unsigned int waitForInt(int tNum){
	unsigned int *vect = (unsigned int *) 0x6FF0;
	state_t *ret_state = (state_t *) INT_OLDAREA;
	termreg_t *dev;
	int number, i;

	if(tNum < 8)
		dev = (termreg_t *) DEV_REG_ADDR(IL_TERMINAL, tNum);
	if(CAUSE_IP_GET(getCAUSE(), INT_TERMINAL)){
	      	if(tNum == SIGNAL){ //function running as callback, ack interrutp and return
			number = *vect;
			for(i = 0; i < 8; i++, number = number >> 1){
				if(number & 1){
					number = i;
					break;
				}
			}
			dev = (termreg_t *) DEV_REG_ADDR(IL_TERMINAL, number);
			dev->transm_command = DEV_C_ACK;
			ret_state->pc -= 4;
			breakfunc();
			LDST(ret_state);
		}
		else if(*vect & (1 << tNum)){ //interrupt had already arrived
			return dev->transm_status;
		}
	} else {
		WAIT();
	}
	return dev->transm_status;
}

void init(){
	state_t *interruptNew = (state_t *) INT_NEWAREA;
	state_t *other_new;

	STST(interruptNew);

	interruptNew->a1 = (unsigned int) SIGNAL;
	interruptNew->pc = (unsigned int) waitForInt;

	other_new = (state_t *) TLB_NEWAREA;
	STST(other_new);
	other_new->pc = (unsigned int) PANIC;
	other_new = (state_t *) PGMTRAP_NEWAREA;
	STST(other_new);
	other_new->pc = (unsigned int) PANIC;
	other_new = (state_t *) SYSBK_NEWAREA;
	STST(other_new);
	other_new->pc = (unsigned int) PANIC;

}

void iprint(char *msg) {

	char *s = msg;
	termreg_t *dev = (termreg_t *) DEV_REG_ADDR(INT_TERMINAL, 0);
	unsigned int status;

	while (*s != '\0') {
		/* Put "transmit char" command+char in term0 register (3rd word). This 
		 *                    actually starts the operation on the device! */
		dev->transm_command = DEV_TTRS_C_TRSMCHAR | (((unsigned int) *s) << 8);

		/* Wait for I/O completion (SYS8) */
		status = waitForInt(0);

		if ((status & DEV_TERM_STATUS) != DEV_TTRS_S_CHARTRSM)
			PANIC();

		if (((status & DEV_TERM_STATUS) >> 8) != *s)
			PANIC();

		s++;    
	}
}

