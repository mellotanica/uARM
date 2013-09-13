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

#ifndef UARM_THUMBISA_CC
#define UARM_THUMBISA_CC

#include "Thumbisa.h"

void Thumbisa::execute(Byte hi, Byte low){
	
	(*this.*main_table[hi][low])();
}

 /* *************** *
  * 				*
  *    Thumb ISA	*
  * 				*
  * *************** */

void Thumbisa::ADD(){
	p->debugARM("ADD (T)");
}

void Thumbisa::ADDH(){
	p->debugARM("ADDH (T)");
}

void Thumbisa::ADDPC(){
	p->debugARM("ADDPC (T)");
}

void Thumbisa::ADDSP(){
	p->debugARM("ADDSP (T)");
}

void Thumbisa::ASR(){
	p->debugARM("ASR (T)");
}

void Thumbisa::B(){
	p->debugARM("B (T)");
}

void Thumbisa::Bcond(){
	p->debugARM("conditional B (T)");
}

void Thumbisa::BL(){
	p->debugARM("BL (T)");
}

void Thumbisa::BX(){
	p->debugARM("BX (T)");
}

void Thumbisa::CMP(){
	p->debugARM("CMP (T)");
}

void Thumbisa::CMPH(){
	p->debugARM("CMPH (T)");
}

void Thumbisa::DP(){	//data processing, triggers an ALU operation
	p->debugARM("DP (T)");
	
	Byte hi = (p->pipeline[PIPELINE_EXECUTE] >> 8) & 3;
	Byte low = (p->pipeline[PIPELINE_EXECUTE] >> 6) & 3;
		(*this.*ALU_table[hi][low])();	//execute ALU operation
}

void Thumbisa::LDMIA(){
	p->debugARM("LDMIA (T)");
}

void Thumbisa::LDR(){
	p->debugARM("LDR (T)");
}

void Thumbisa::LDRB(){
	p->debugARM("LDRB (T)");
}

void Thumbisa::LDRH(){
	p->debugARM("LDRH (T)");
}

void Thumbisa::LDRPC(){
	p->debugARM("LDRPC (T)");
}

void Thumbisa::LDRSB(){
	p->debugARM("LDRSB (T)");
}

void Thumbisa::LDRSH(){
	p->debugARM("LDRSH (T)");
}

void Thumbisa::LDRSP(){
	p->debugARM("LDRSP (T)");
}

void Thumbisa::LSL(){
	p->debugARM("LSL (T)");
}

void Thumbisa::LSR(){
	p->debugARM("LSR (T)");
}

void Thumbisa::MOV(){
	p->debugARM("MOV (T)");
}

void Thumbisa::MOVH(){
	p->debugARM("MOVH (T)");
}

void Thumbisa::POP(){
	p->debugARM("POP (T)");
}

void Thumbisa::PUSH(){
	p->debugARM("PUSH (T)");
}

void Thumbisa::STMIA(){
	p->debugARM("STMIA (T)");
}

void Thumbisa::STR(){
	p->debugARM("STR (T)");
}

void Thumbisa::STRB(){
	p->debugARM("STRB (T)");
}

void Thumbisa::STRH(){
	p->debugARM("STRH (T)");
}

void Thumbisa::STRSP(){
	p->debugARM("STRSP (T)");
}

void Thumbisa::SUB(){
	p->debugARM("SUB (T)");
}

void Thumbisa::SWI(){
	p->debugARM("SWI (T)");
	
		p->softwareInterruptTrap();
}

void Thumbisa::UND(){
	p->debugARM("UND (T)");
	
		p->undefinedTrap();
}

/*
 * ALU operations
 */

void Thumbisa::ADD_ALU(){
	p->debugARM("ADD_ALU (T)");
}

void Thumbisa::AND_ALU(){
	p->debugARM("AND_ALU (T)");
}

void Thumbisa::ASR_ALU(){
	p->debugARM("ASR_ALU (T)");
}

void Thumbisa::BIC_ALU(){
	p->debugARM("BIC_ALU (T)");
}

void Thumbisa::CMN_ALU(){
	p->debugARM("CMN_ALU (T)");
}

void Thumbisa::CMP_ALU(){
	p->debugARM("CMP_ALU (T)");
}

void Thumbisa::EOR_ALU(){
	p->debugARM("EOR_ALU (T)");
}

void Thumbisa::LSL_ALU(){
	p->debugARM("LSL_ALU (T)");
}

void Thumbisa::LSR_ALU(){
	p->debugARM("LSR_ALU (T)");
}

void Thumbisa::MUL_ALU(){
	p->debugARM("MUL_ALU (T)");
}

void Thumbisa::MVN_ALU(){
	p->debugARM("MVN_ALU (T)");
}

void Thumbisa::NEG_ALU(){
	p->debugARM("NEG_ALU (T)");
}

void Thumbisa::ORR_ALU(){
	p->debugARM("ORR_ALU (T)");
}

void Thumbisa::ROR_ALU(){
	p->debugARM("ROR_ALU (T)");
}

void Thumbisa::SUB_ALU(){
	p->debugARM("SUB_ALU (T)");
}

void Thumbisa::TST_ALU(){
	p->debugARM("TST_ALU (T)");	
}

#endif //UARM_THUMBISA_CC
