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
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::ADDH(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::ADDPC(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::ADDSP(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::ASR(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::B(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::Bcond(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::BL(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::BX(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::CMP(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::CMPH(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::DP(){	//data processing, triggers an ALU operation
	p->debugThumb(__FUNCTION__);
	
	Byte hi = (p->pipeline[PIPELINE_EXECUTE] >> 8) & 3;
	Byte low = (p->pipeline[PIPELINE_EXECUTE] >> 6) & 3;
		(*this.*ALU_table[hi][low])();	//execute ALU operation
}

void Thumbisa::LDMIA(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::LDR(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::LDRB(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::LDRH(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::LDRPC(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::LDRSB(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::LDRSH(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::LDRSP(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::LSL(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::LSR(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::MOV(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::MOVH(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::POP(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::PUSH(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::STMIA(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::STR(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::STRB(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::STRH(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::STRSP(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::SUB(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::SWI(){
	p->debugThumb(__FUNCTION__);
	
		p->softwareInterruptTrap();
}

void Thumbisa::UND(){
	p->debugThumb(__FUNCTION__);
	
		p->undefinedTrap();
}

/*
 * ALU operations
 */

void Thumbisa::ADD_ALU(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::AND_ALU(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::ASR_ALU(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::BIC_ALU(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::CMN_ALU(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::CMP_ALU(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::EOR_ALU(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::LSL_ALU(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::LSR_ALU(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::MUL_ALU(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::MVN_ALU(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::NEG_ALU(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::ORR_ALU(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::ROR_ALU(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::SUB_ALU(){
	p->debugThumb(__FUNCTION__);
}

void Thumbisa::TST_ALU(){
	p->debugThumb(__FUNCTION__);	
}

#endif //UARM_THUMBISA_CC
