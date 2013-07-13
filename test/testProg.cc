#include "machine.h"
#include "services/debugger.cc"
#include "const.h"

void writeWord(ramMemory *ram, Word *word, Word addr){
	ram->writeW(&addr, *word, false);
}

#define DBRUN 1
//must be word-aligned
Word baseAddr = 0;
Word proglen = 14;
Word program[] = {
	0xE3A03003,	//MOV		r3, #3
	0xE3B02003,	//MOVs		r2, #3
	0x10531002,	//SUBs NE	r1, r3, r2
	0x0281120F,	//ADD EQ	r1, r1, #0xF0000000
	0xE3A024FF,	//MOV		r2, #0xFF000000
	0xE0913002,	//ADDs		r3, r1, r2
	0x61530001,	//CMPs VS	r3, r1
	0x21520001,	//CMPs CS	r2, r1
	0xE3A0D804, //MOV		sp, #40000
	0xE48D3004,	//STR		r3, [sp, #4]
	0xE3A03000,	//MOV		r3, #0
	0xE53D4004,	//LDR		r4, [sp, #-4]!
	0xE5BD5001,	//LDR		r5, [sp, #1]!
	OP_HALT,
 };

/* main() {
 * 		int a, b, s;
 * 		a = 1;
 * 		b = 3;
 * 		s = a+b;
 * }
 
Word program[] = {	0xE3A0D804, //MOV	sp, #40000
					0xE52DB004,	//STR	fp, [sp, #-4]!
					0xE28DB000,	//ADD	fp, sp, #0
					0xE24DD014,	//SUB	sp, sp, #20
					0xE3A03001,	//MOV	r3, #1
					0xE50B3010,	//STR	r3, [fp, #-16]
					0xE3A03003,	//MOV	r3, #3
					0xE50B300C,	//STR	r3, [fp, #-12]
					0xE51B2010,	//LDR	r2, [fp, #-16]
					0xE51B300C,	//LDR	r3, [fp, #-12]
					0xE0823003,	//ADD	r3, r2, r3
					0xE50B3008,	//STR	r3, [fp, #-8]
					0xE28BD000,	//ADD	sp, fp, #0
					0xE8BD0800,	//LDMFD	sp!, {fp}
					0xE12FFF1E,	//BX	lr
					OP_HALT,
				 };
*/ 
void runcycle(machine *mac, bool debugRun){
	bool speedrun = true;
	char read;
	cout << "Run program: press return for next step, type f and press return for quick execution\n";
	int c = 0;
	while(debugRun && (mac->getCPU()->getStatus() != PS_HALTED) && c <= proglen+4){
		cin.get(read);
		if(read == 'f'){
			speedrun = true;
			break;
		} else
			mac->step();
		cout << "step..\n";
		c++;
		printStatus(mac);
	}
	if(speedrun)
		mac->run();
	printStatus(mac);
}

main(int argc, int* argv){
	Word size = MEM_SIZE_W;
	bool debugRun = DBRUN;
	
	cout << "TEST START, CREATING MACHINE..\n\n";
	machine* mac = new machine(size);
	cout << "\n INIT COMPLETED!\n\n----\n\n";
	
	cout << "filling memory...\n";
	for(int i = 0; i < proglen; i++)
		writeWord(mac->getBus()->getRam(), &program[i], (i*4)+baseAddr);
	
	
	cout << "read first byte: ";
	Byte b = mac->getBus()->getRam()->read(&baseAddr);
	
	debugByte(&b);
	
	cout << "\nread first word: ";
	Word w = mac->getBus()->getRam()->readW(&baseAddr);
	printRegister("",&w);
	
	cout << "\nmac->BUS: " << mac->getBus();
	
	cout << "\nDONE, start execution\n";
	
	runcycle(mac, debugRun);
	
	return 0;
}
