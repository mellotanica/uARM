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
	
    void execute(Byte hi, Byte low, HalfWord instr);
	
private:
	
	processor *p;
	
    Word shift(Word src, Word amount, Byte type);

    typedef void(Thumbisa::*InstrPointer)(HalfWord instr);
    typedef void(Thumbisa::*AluPointer)(Word *rd, Word *rs);
	
    void ADD(HalfWord instr);
    void ADDH(HalfWord instr);
    void ADDPC(HalfWord instr);
    void ADDSP(HalfWord instr);
    void ASR(HalfWord instr);
    void B(HalfWord instr);
    void Bcond(HalfWord instr);
    void BL(HalfWord instr);
    void BX(HalfWord instr);
    void CMP(HalfWord instr);
    void CMPH(HalfWord instr);
    void DP(HalfWord instr);
    void LDMIA(HalfWord instr);
    void LDR(HalfWord instr);
    void LDRB(HalfWord instr);
    void LDRH(HalfWord instr);
    void LDRPC(HalfWord instr);
    void LDRSB(HalfWord instr);
    void LDRSH(HalfWord instr);
    void LDRSP(HalfWord instr);
    void LSL(HalfWord instr);
    void LSR(HalfWord instr);
    void MOV(HalfWord instr);
    void MOVH(HalfWord instr);
    void POP(HalfWord instr);
    void PUSH(HalfWord instr);
    void STMIA(HalfWord instr);
    void STR(HalfWord instr);
    void STRB(HalfWord instr);
    void STRH(HalfWord instr);
    void STRSP(HalfWord instr);
    void SUB(HalfWord instr);
    void SWI(HalfWord instr);
    void UND(HalfWord instr);
	
    void ADC_ALU(Word *rd, Word *rs);
    void AND_ALU(Word *rd, Word *rs);
    void ASR_ALU(Word *rd, Word *rs);
    void BIC_ALU(Word *rd, Word *rs);
    void CMN_ALU(Word *rd, Word *rs);
    void CMP_ALU(Word *rd, Word *rs);
    void EOR_ALU(Word *rd, Word *rs);
    void LSL_ALU(Word *rd, Word *rs);
    void LSR_ALU(Word *rd, Word *rs);
    void MUL_ALU(Word *rd, Word *rs);
    void MVN_ALU(Word *rd, Word *rs);
    void NEG_ALU(Word *rd, Word *rs);
    void ORR_ALU(Word *rd, Word *rs);
    void ROR_ALU(Word *rd, Word *rs);
    void SBC_ALU(Word *rd, Word *rs);
    void TST_ALU(Word *rd, Word *rs);
	
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
	
    const AluPointer ALU_table[4][4] = {
		{&Thumbisa::AND_ALU, &Thumbisa::EOR_ALU, &Thumbisa::LSL_ALU, &Thumbisa::LSR_ALU},
        {&Thumbisa::ASR_ALU, &Thumbisa::ADC_ALU, &Thumbisa::SBC_ALU, &Thumbisa::ROR_ALU},
		{&Thumbisa::TST_ALU, &Thumbisa::NEG_ALU, &Thumbisa::CMP_ALU, &Thumbisa::CMN_ALU},
		{&Thumbisa::ORR_ALU, &Thumbisa::MUL_ALU, &Thumbisa::BIC_ALU, &Thumbisa::MVN_ALU}
	};
};

#endif //UARM_THUMBISA_H
