#include "machine.h"
#include "services/debugger.cc"
#include "const.h"

void printWord(string head, Word reg) {
	cout << head << "0x" << hex << uppercase << setw(8) << setfill('0') << reg << nouppercase << dec;
}

void writeWord(ramMemory *ram, Word *word, Word addr){
	ram->writeW(&addr, *word, false);
}

Word readWord(ramMemory *ram, Word addr){
	return ram->readW(&addr, false);
}

#define DBRUN 1
//must be word-aligned
Word baseAddr = 0;
Word proglen = 43;
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
	0xE3A014FA,	//MOV		r1, #0xFA000000
	0xE38138FB,	//ORR		r3, r1, #0x00FB0000
	0xE3831CFC,	//ORR		r1, r3, #0x0000FC00
	0xE38130FD,	//ORR		r3, r1, #0x000000FD
	0xE16D30B1,	//STRHW		r3, [sp, #-1]!
	0xE1DD10B0,	//LDRH		r1, [sp, #0]!
	0xE0DD20F0,	//LDRSH		r2, [sp, #0]
	0xE0DD40D0,	//LDRSB		r4, [sp, #0]
	0xE3A010FF,	//MOV		r1, #0x000000FF
	0xE3813C0F,	//ORR		r3, r1, #0x00000F00
	0xE14D30B0,	//STRH		r3, [sp, #0]!
	0xE0DD10F0,	//LDRSH		r1	[sp, #0]
	0xE0DD20D0,	//LDRSB		r2	[sp, #0]
	0xE3A04003,	//MOV		r4, #3
	0xE3A03003,	//MOV		r3, #3
	0xE2532001, //SUBs		r2, r3, #1
	0xE1A03002,	//MOV		r3, r2
	0x3BFFFFFD,	//BL CC		#-12
	0xE2441001,	//SUB		r1, r4, #1
	0xE1A04001,	//MOV		r4, r1
	0xE3540002,	//CMP		r4, #2
	0x0AFFFFFC,	//B	EQ		#-16
	0xE3540001,	//CMP		r4, #1
	0xE24F5027,	//SUB		r5, PC, #39 
	0x012FFF15,	//BX EQ		r5
	0xE28F5008,	//ADD		r5, PC, #8
	0xE12FFF15,	//BX AL		r5
	0xE3A01028,	//MOV		r1, #40
	0xE3A01004,	//MOV		r1, #4
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
	while(debugRun && (mac->getCPU()->getStatus() != PS_HALTED)){
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

main(int argc, char** argv){
	Word size = MEM_SIZE_W;
	bool debugRun = DBRUN;
	
	cout << "TEST START, CREATING MACHINE..\n\n";
	machine* mac = new machine(size);
	
	cout << "FILLING MEMORY...\n";
	for(int i = 0; i < proglen; i++){
		writeWord(mac->getBus()->getRam(), &program[i], baseAddr+(i*4));
		cout << "written prog[" <<i<< "]: ";
		printWord("",readWord(mac->getBus()->getRam(), baseAddr+(i*4)));
		cout << "\n";
	}
	
	cout << "PREPARING PC..\n";
	mac->init();
	
	cout << "\n INIT COMPLETED!\n\n----\n\n";
	
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
