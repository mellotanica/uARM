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
 
#ifndef UARM_ARMISA_CC
#define UARM_ARMISA_CC
 
#include "ARMisa.h"

void ARMisa::execute(Byte hi, Byte low){
	if(p->condCheck()){
		(*this.*ARM_table[hi][low])();	//execute funcion from ARM_table in cell instr[27:20][7:4]
	} else
		p->NOP();
}

 /* *************** *
  * 				*
  * 	ARM ISA		*
  * 				*
  * *************** */

void ARMisa::ADC(){
	p->debugARM(__FUNCTION__);
	
		p->dataProcessing(5);
}

void ARMisa::ADD(){
	p->debugARM("ADD");
	
		p->dataProcessing(4);
}

void ARMisa::AND(){
	p->debugARM("AND");
	
		p->dataProcessing(0);
}

void ARMisa::B(){
	p->debugARM("B");
	
		p->branch(false, false);
}

void ARMisa::BIC(){
	p->debugARM("BIC");
	
		p->dataProcessing(14);
}

void ARMisa::BL(){
	p->debugARM("BL");
	
		p->branch(true, false);
}

void ARMisa::BX(){
	p->debugARM("BX");
	
		p->branch(false, true);
}

void ARMisa::CDP(){
	p->debugARM("CDP");
	
		p->coprocessorOperation();
}

void ARMisa::CMN(){
	p->debugARM("CMN");
	
		p->dataProcessing(11);
}

void ARMisa::CMP(){
	p->debugARM("CMP");
	
		p->dataProcessing(10);
}

void ARMisa::EOR(){
	p->debugARM("EOR");
	
		p->dataProcessing(1);
}

void ARMisa::LDC(){
	p->debugARM("LDC");
	
		p->coprocessorTransfer(true, true);
}

void ARMisa::LDM(){
	p->debugARM("LDM");
	
		p->blockDataTransfer(true);
}

void ARMisa::LDR(){
	p->debugARM("LDR");
	
		p->singleMemoryAccess(true);
}

void ARMisa::LDRH(){
	p->debugARM("LDRH");
	
		p->halfwordDataTransfer(false, true);
}

void ARMisa::LDRSB(){
	p->debugARM("LDRSB");
		
		p->halfwordDataTransfer(true, false);
}

void ARMisa::LDRSH(){
	p->debugARM("LDRSH");
	
		p->halfwordDataTransfer(true, true);
}

void ARMisa::MCR(){
	p->debugARM("MCR");
	
		p->coprocessorTransfer(false, true);
}

void ARMisa::MLA(){
	p->debugARM("MLA");
	
		p->multiply(true, false);
}

void ARMisa::MLAL(){
	p->debugARM("MLAL");
	
		p->multiply(true, true);
}

void ARMisa::MOV(){
	p->debugARM("MOV");
	
		p->dataProcessing(13);
}

void ARMisa::MRC(){
	p->debugARM("MRC");
	
		p->coprocessorTransfer(false, false);
}

void ARMisa::MRS(){
	p->debugARM("MRS");
	
		p->accessPSR(true);
}

void ARMisa::MSR(){
	p->debugARM("MSR");
	
		p->accessPSR(false);
}

void ARMisa::MUL(){
	p->debugARM("MUL");
	
		p->multiply(false, false);
}

void ARMisa::MULL(){
	p->debugARM("MULL");
	
		p->multiply(false, true);
}

void ARMisa::MVN(){
	p->debugARM("MVN");
	
		p->dataProcessing(15);
}

void ARMisa::ORR(){
	p->debugARM("ORR");
	
		p->dataProcessing(12);
}

void ARMisa::RSB(){
	p->debugARM("RSB");
	
		p->dataProcessing(3);
}

void ARMisa::RSC(){
	p->debugARM("RSC");
	
		p->dataProcessing(7);
}

void ARMisa::SBC(){
	p->debugARM("SBC");
	
		p->dataProcessing(6);
}

void ARMisa::STC(){
	p->debugARM("STC");
	
		p->coprocessorTransfer(true, false);
}

void ARMisa::STM(){
	p->debugARM("STM");
	
		p->blockDataTransfer(false);
}

void ARMisa::STR(){
	p->debugARM("STR");
	
		p->singleMemoryAccess(false);
}

void ARMisa::STRH(){
	p->debugARM("STRH");
	
		p->halfwordDataTransfer(false, false);
}

void ARMisa::SUB(){
	p->debugARM("SUB");
	
		p->dataProcessing(2);
}

void ARMisa::SWI(){
	p->debugARM("SWI");
	
		p->softwareInterruptTrap();
}

void ARMisa::SWP(){
	p->debugARM("SWP");
	
		p->singleDataSwap();
}

void ARMisa::TEQ(){
	p->debugARM("TEQ");
	
		p->dataProcessing(9);
}

void ARMisa::TST(){
	p->debugARM("TST");
	
		p->dataProcessing(8);
}

void ARMisa::UND(){
	p->debugARM("UND");
	
		p->undefinedTrap();
}

#endif //UARM_ARMISA_CC
