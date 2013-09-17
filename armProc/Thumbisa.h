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
 
/* 
 * This header file provides Thumb instruction set decode and execution facilities
 */
 
#ifndef UARM_THUMBISA_H
#define UARM_THUMBISA_H

#include "processor.h"

class Thumbisa{
public:
    Thumbisa(processor *father){p = father;}
    ~Thumbisa(){}
	
	void execute(Byte hi, Byte low);
	
private:
	
	processor *p;
	
	typedef void(Thumbisa::*InstrPointer)();
	
	void ADD();
	void ADDH();
	void ADDPC();
	void ADDSP();
	void ASR();
	void B();
	void Bcond();
	void BL();
	void BX();
	void CMP();
	void CMPH();
	void DP();
	void LDMIA();
	void LDR();
	void LDRB();
	void LDRH();
	void LDRPC();
	void LDRSB();
	void LDRSH();
	void LDRSP();
	void LSL();
	void LSR();
	void MOV();
	void MOVH();
	void POP();
	void PUSH();
	void STMIA();
	void STR();
	void STRB();
	void STRH();
	void STRSP();
	void SUB();
	void SWI();
	void UND();
	
	void ADD_ALU();
	void AND_ALU();
	void ASR_ALU();
	void BIC_ALU();
	void CMN_ALU();
	void CMP_ALU();
	void EOR_ALU();
	void LSL_ALU();
	void LSR_ALU();
	void MUL_ALU();
	void MVN_ALU();
	void NEG_ALU();
	void ORR_ALU();
	void ROR_ALU();
	void SUB_ALU();
	void TST_ALU();
	
	const InstrPointer main_table[16][16] = {
		{&Thumbisa::LSL, &Thumbisa::LSL, &Thumbisa::LSL, &Thumbisa::LSL, &Thumbisa::LSL, &Thumbisa::LSL, &Thumbisa::LSL, &Thumbisa::LSL, &Thumbisa::LSR, &Thumbisa::LSR, &Thumbisa::LSR, &Thumbisa::LSR, &Thumbisa::LSR, &Thumbisa::LSR, &Thumbisa::LSR, &Thumbisa::LSR},
		{&Thumbisa::ASR, &Thumbisa::ASR, &Thumbisa::ASR, &Thumbisa::ASR, &Thumbisa::ASR, &Thumbisa::ASR, &Thumbisa::ASR, &Thumbisa::ASR, &Thumbisa::ADD, &Thumbisa::ADD, &Thumbisa::SUB, &Thumbisa::SUB, &Thumbisa::ADD, &Thumbisa::ADD, &Thumbisa::SUB, &Thumbisa::SUB},
		{&Thumbisa::MOV, &Thumbisa::MOV, &Thumbisa::MOV, &Thumbisa::MOV, &Thumbisa::MOV, &Thumbisa::MOV, &Thumbisa::MOV, &Thumbisa::MOV, &Thumbisa::CMP, &Thumbisa::CMP, &Thumbisa::CMP, &Thumbisa::CMP, &Thumbisa::CMP, &Thumbisa::CMP, &Thumbisa::CMP, &Thumbisa::CMP},
		{&Thumbisa::ADD, &Thumbisa::ADD, &Thumbisa::ADD, &Thumbisa::ADD, &Thumbisa::ADD, &Thumbisa::ADD, &Thumbisa::ADD, &Thumbisa::ADD, &Thumbisa::SUB, &Thumbisa::SUB, &Thumbisa::SUB, &Thumbisa::SUB, &Thumbisa::SUB, &Thumbisa::SUB, &Thumbisa::SUB, &Thumbisa::SUB},
		{&Thumbisa::DP, &Thumbisa::DP, &Thumbisa::DP, &Thumbisa::DP, &Thumbisa::ADDH, &Thumbisa::CMPH, &Thumbisa::MOVH, &Thumbisa::BX, &Thumbisa::LDRPC, &Thumbisa::LDRPC, &Thumbisa::LDRPC, &Thumbisa::LDRPC, &Thumbisa::LDRPC, &Thumbisa::LDRPC, &Thumbisa::LDRPC, &Thumbisa::LDRPC},
		{&Thumbisa::STR, &Thumbisa::STR, &Thumbisa::STRH, &Thumbisa::STRH, &Thumbisa::STRB, &Thumbisa::STRB, &Thumbisa::LDRSB, &Thumbisa::LDRSB, &Thumbisa::LDR, &Thumbisa::LDR, &Thumbisa::LDRH, &Thumbisa::LDRH, &Thumbisa::LDRB, &Thumbisa::LDRB, &Thumbisa::LDRSH, &Thumbisa::LDRSH},
		{&Thumbisa::STR, &Thumbisa::STR, &Thumbisa::STR, &Thumbisa::STR, &Thumbisa::STR, &Thumbisa::STR, &Thumbisa::STR, &Thumbisa::STR, &Thumbisa::LDR, &Thumbisa::LDR, &Thumbisa::LDR, &Thumbisa::LDR, &Thumbisa::LDR, &Thumbisa::LDR, &Thumbisa::LDR, &Thumbisa::LDR},
		{&Thumbisa::STRB, &Thumbisa::STRB, &Thumbisa::STRB, &Thumbisa::STRB, &Thumbisa::STRB, &Thumbisa::STRB, &Thumbisa::STRB, &Thumbisa::STRB, &Thumbisa::LDRB, &Thumbisa::LDRB, &Thumbisa::LDRB, &Thumbisa::LDRB, &Thumbisa::LDRB, &Thumbisa::LDRB, &Thumbisa::LDRB, &Thumbisa::LDRB},
		{&Thumbisa::STRH, &Thumbisa::STRH, &Thumbisa::STRH, &Thumbisa::STRH, &Thumbisa::STRH, &Thumbisa::STRH, &Thumbisa::STRH, &Thumbisa::STRH, &Thumbisa::LDRH, &Thumbisa::LDRH, &Thumbisa::LDRH, &Thumbisa::LDRH, &Thumbisa::LDRH, &Thumbisa::LDRH, &Thumbisa::LDRH, &Thumbisa::LDRH},
		{&Thumbisa::STRSP, &Thumbisa::STRSP, &Thumbisa::STRSP, &Thumbisa::STRSP, &Thumbisa::STRSP, &Thumbisa::STRSP, &Thumbisa::STRSP, &Thumbisa::STRSP, &Thumbisa::LDRSP, &Thumbisa::LDRSP, &Thumbisa::LDRSP, &Thumbisa::LDRSP, &Thumbisa::LDRSP, &Thumbisa::LDRSP, &Thumbisa::LDRSP, &Thumbisa::LDRSP},
		{&Thumbisa::ADDPC, &Thumbisa::ADDPC, &Thumbisa::ADDPC, &Thumbisa::ADDPC, &Thumbisa::ADDPC, &Thumbisa::ADDPC, &Thumbisa::ADDPC, &Thumbisa::ADDPC, &Thumbisa::ADDSP, &Thumbisa::ADDSP, &Thumbisa::ADDSP, &Thumbisa::ADDSP, &Thumbisa::ADDSP, &Thumbisa::ADDSP, &Thumbisa::ADDSP, &Thumbisa::ADDSP},
		{&Thumbisa::ADDSP, &Thumbisa::UND, &Thumbisa::UND, &Thumbisa::UND, &Thumbisa::PUSH, &Thumbisa::PUSH, &Thumbisa::UND, &Thumbisa::UND, &Thumbisa::UND, &Thumbisa::UND, &Thumbisa::UND, &Thumbisa::UND, &Thumbisa::POP, &Thumbisa::POP, &Thumbisa::UND, &Thumbisa::UND},
		{&Thumbisa::STMIA, &Thumbisa::STMIA, &Thumbisa::STMIA, &Thumbisa::STMIA, &Thumbisa::STMIA, &Thumbisa::STMIA, &Thumbisa::STMIA, &Thumbisa::STMIA, &Thumbisa::LDMIA, &Thumbisa::LDMIA, &Thumbisa::LDMIA, &Thumbisa::LDMIA, &Thumbisa::LDMIA, &Thumbisa::LDMIA, &Thumbisa::LDMIA, &Thumbisa::LDMIA},
		{&Thumbisa::Bcond, &Thumbisa::Bcond, &Thumbisa::Bcond, &Thumbisa::Bcond, &Thumbisa::Bcond, &Thumbisa::Bcond, &Thumbisa::Bcond, &Thumbisa::Bcond, &Thumbisa::Bcond, &Thumbisa::Bcond, &Thumbisa::Bcond, &Thumbisa::Bcond, &Thumbisa::Bcond, &Thumbisa::Bcond, &Thumbisa::UND, &Thumbisa::SWI},
		{&Thumbisa::B, &Thumbisa::B, &Thumbisa::B, &Thumbisa::B, &Thumbisa::B, &Thumbisa::B, &Thumbisa::B, &Thumbisa::B, &Thumbisa::UND, &Thumbisa::UND, &Thumbisa::UND, &Thumbisa::UND, &Thumbisa::UND, &Thumbisa::UND, &Thumbisa::UND, &Thumbisa::UND},
		{&Thumbisa::BL, &Thumbisa::BL, &Thumbisa::BL, &Thumbisa::BL, &Thumbisa::BL, &Thumbisa::BL, &Thumbisa::BL, &Thumbisa::BL, &Thumbisa::BL, &Thumbisa::BL, &Thumbisa::BL, &Thumbisa::BL, &Thumbisa::BL, &Thumbisa::BL, &Thumbisa::BL, &Thumbisa::BL}
	};
	
	const InstrPointer ALU_table[4][4] = {
		{&Thumbisa::AND_ALU, &Thumbisa::EOR_ALU, &Thumbisa::LSL_ALU, &Thumbisa::LSR_ALU},
		{&Thumbisa::ASR_ALU, &Thumbisa::ADD_ALU, &Thumbisa::SUB_ALU, &Thumbisa::ROR_ALU},
		{&Thumbisa::TST_ALU, &Thumbisa::NEG_ALU, &Thumbisa::CMP_ALU, &Thumbisa::CMN_ALU},
		{&Thumbisa::ORR_ALU, &Thumbisa::MUL_ALU, &Thumbisa::BIC_ALU, &Thumbisa::MVN_ALU}
	};
};

#endif //UARM_THUMBISA_H
