/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uARM
 *
 * Copyright (C) 2013 Marco Melletti
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
#include <iostream>
#include <bitset>
#include <sstream>
#include <iomanip>
#include <stdlib.h>

#ifndef UARM_DEBUGGER
#define UARM_DEBUGGER

#define DEBUGGER_ON 1
//#define DEBUGGER_ON 0

using namespace std;

void printCP15Reg(cp15 *cp);

void aletr(string text) {
	#if DEBUGGER_ON>0
	cout << text;
	#endif
}

void alert(string head, int count, string tail) {
	#if DEBUGGER_ON>0
	cout << head << count << tail;
	#endif
}

void debugByte(Byte *b){
	if(b > 0){
		bitset<8> val(*b);
		cout << val;
	}
	else
		cout << "UNDEF";
}

void printByte(Byte *b){
if(b > 0) {
		unsigned val = (unsigned) *b;
		cout << "0x" << hex << uppercase << setw(2) << setfill('0') << val << nouppercase << dec;
	} else
		cout << "UNDEF";
	
}

void debugRegister(Word *w){
	#if DEBUGGER_ON>0
	if(w > 0){
		bitset<8> val1(*w>>24);
		bitset<8> val2(*w>>16);
		bitset<8> val3(*w>>8);
		bitset<8> val4(*w);
		cout << val1 << " " << val2 << " " << val3 << " " << val4;
	} else
		cout << "UNDEF";
	#endif
}

void debugRegister(string head, Word *reg, string tail) {
	#if DEBUGGER_ON>0
	cout << head;
	debugRegister(reg);
	cout << tail;
	#endif
}

void debugRegister(string head, Word *reg) {
	#if DEBUGGER_ON>0
	debugRegister(head, reg, "\n");
	#endif
}

void printRegister(string head, Word *reg) {
	#if DEBUGGER_ON>0
	if(reg > 0)
		cout << head << "0x" << hex << uppercase << setw(8) << setfill('0') << *reg << nouppercase << dec;
	else
		cout << head << "UNDEF     ";
	#endif
}

void printHalfWord(string head, HalfWord *wd) {
	#if DEBUGGER_ON>0
	if(wd > 0)
		cout << head << "0x" << hex << uppercase << setw(4) << setfill('0') << *wd << nouppercase << dec;
	else
		cout << head << "UNDEF     ";
	#endif
}

void printOP(pu *unit){
	#if DEBUGGER_ON>0
	cout << "\nExecuting "<<unit->mnemonicOPcode<<" : ";
	if(unit->isOPcodeARM)
		printRegister("(ARM) ", &(unit->OPcode));
	else{
		HalfWord hw = (HalfWord) unit->OPcode;
		printHalfWord("(Thumb) ", &hw);
	}
	#endif
}

void printPipeline(pu *unit){
	#if DEBUGGER_ON>0
	cout << "\n\nPipeline:\tFetch\t\tDecode\t\tExecute\n";
	printRegister("\t\t",unit->getPipeline(PIPELINE_FETCH));
	printRegister("\t",unit->getPipeline(PIPELINE_DECODE));
	printRegister("\t",unit->getPipeline(PIPELINE_EXECUTE));
	printOP(unit);
	#endif
}


void printCP15Reg(cp15 *cp){
	#if DEBUGGER_ON>0
	cout << "\n###########################################################\n";
	cout << "#                   CP15    Status                        #\n";
	cout << "###########################################################";
	
	printPipeline(cp);
	
	cout << "\n\nRegisters:";

	unsigned int cols = 4;
	unsigned int limit = CP15_REGISTERS_NUM/cols;
	limit += (CP15_REGISTERS_NUM%cols == 0 ? 0 : 1);

	for(unsigned int i = 0; i < limit; i++){
		for(unsigned int j = 0; j < cols; j++){
			unsigned int el = i+(j*limit);
			if(el < CP15_REGISTERS_NUM){
				cout << "\tr" << el << ":";
				printRegister("\t", cp->getRegister(j));
			} else
				break;
		}
		cout << "\n\t";
	}
	#endif
}

void printCPUReg(processor *cpu){
	#if DEBUGGER_ON>0
	cout << "\n###########################################################\n";
	cout << "#                   CPU     Status                        #\n";
	cout << "###########################################################";
	
	printPipeline(cpu);
	
	cout << "\n\nRegisters:\tUsr/Sys\t\tSvc\t\tAbt\t\tUndef\t\tIrq\t\tFiq\n";

	printRegister("r0\t\t",cpu->getRegister(0));cout << "\t\t\t\t\t\t\t\t\t\t\tr0";

	printRegister("\nr1\t\t",cpu->getRegister(1));cout << "\t\t\t\t\t\t\t\t\t\t\tr1";
	
	printRegister("\nr2\t\t",cpu->getRegister(2));cout << "\t\t\t\t\t\t\t\t\t\t\tr2";
	
	printRegister("\nr3\t\t",cpu->getRegister(3));cout << "\t\t\t\t\t\t\t\t\t\t\tr3";
	
	printRegister("\nr4\t\t",cpu->getRegister(4));cout << "\t\t\t\t\t\t\t\t\t\t\tr4";
	
	printRegister("\nr5\t\t",cpu->getRegister(5));cout << "\t\t\t\t\t\t\t\t\t\t\tr5";
	
	printRegister("\nr6\t\t",cpu->getRegister(6));cout << "\t\t\t\t\t\t\t\t\t\t\tr6";
	
	printRegister("\nr7\t\t",cpu->getRegister(7));cout << "\t\t\t\t\t\t\t\t\t\t\tr7";
	
	printRegister("\nr8\t\t",cpu->getRegister(8));printRegister("\t\t\t\t\t\t\t\t\t",cpu->getRegister(17));cout << "\tr8";
	
	printRegister("\nr9\t\t",cpu->getRegister(9));printRegister("\t\t\t\t\t\t\t\t\t",cpu->getRegister(18));cout << "\tr9";
	
	printRegister("\nr10\t\t",cpu->getRegister(10));printRegister("\t\t\t\t\t\t\t\t\t",cpu->getRegister(19));cout << "\tr10";
	
	printRegister("\nr11\t\t",cpu->getRegister(11));printRegister("\t\t\t\t\t\t\t\t\t",cpu->getRegister(20));cout << "\tr11";
	
	printRegister("\nr12\t\t",cpu->getRegister(12));printRegister("\t\t\t\t\t\t\t\t\t",cpu->getRegister(21));cout << "\tr12";
	
	printRegister("\nr13(SP)\t\t",cpu->getRegister(13));
	printRegister("\t",cpu->getRegister(25));
	printRegister("\t",cpu->getRegister(28));
	printRegister("\t",cpu->getRegister(34));
	printRegister("\t",cpu->getRegister(31));
	printRegister("\t",cpu->getRegister(22));
	cout << "\tr13";
	
	printRegister("\nr14(LR)\t\t",cpu->getRegister(14));
	printRegister("\t",cpu->getRegister(26));
	printRegister("\t",cpu->getRegister(29));
	printRegister("\t",cpu->getRegister(35));
	printRegister("\t",cpu->getRegister(32));
	printRegister("\t",cpu->getRegister(23));
	cout << "\tr14";
	
	printRegister("\nr15(PC)\t\t",cpu->getRegister(15));cout << "\t\t\t\t\t\t\t\t\t\t\tr15";
	
	printRegister("\nCPSR\t\t",cpu->getRegister(16));cout << "\t\t\t\t\t\t\t\t\t\t\tCPSR";
	
	printRegister("\nSPSR\t\t\t\t",cpu->getRegister(27));
	printRegister("\t",cpu->getRegister(30));
	printRegister("\t",cpu->getRegister(36));
	printRegister("\t",cpu->getRegister(33));
	printRegister("\t",cpu->getRegister(34));
	cout << "\tSPSR";
	#endif
}



void printBus(systemBus *bus){
	#if DEBUGGER_ON>0
	cout << "\n###########################################################\n";
	cout << "#                   BUS    Status                        #\n";
	cout << "###########################################################";
	
	printRegister("\n\nCurrent Fetch:\t", &bus->currentFetch);
	#endif
}

void printStatus(machine *mac){
	#if DEBUGGER_ON>0
	system("clear");
	printCPUReg(mac->getCPU());
	printCP15Reg((cp15 *)mac->getCoprocessor(15));
	printBus(mac->getBus());
	cout << "\n###########################################################\n";
	#endif
}

#endif //UARM_DEBUGGER
