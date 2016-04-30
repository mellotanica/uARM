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

#ifndef UARM_ARMISA_H
#define UARM_ARMISA_H

#include "armProc/processor.h"

class ARMisa{
public:
    ARMisa(processor *father){p = father;}
    ~ARMisa(){}

	void execute(Byte hi, Byte low);

private:
	processor *p;

    typedef void(ARMisa::*InstrPointer)();

	//ARM ISA

    void ADC();	//add with carry
    void ADD();	//add
    void AND();	//AND
    void B();	//brach
    void BIC();	//bit clear
    void BL();	//branch with link
    void BX();	//branch and exchange
    void CDP();	//coprocessor data processing
    void CMN(); //compare negative
    void CMP(); //compare
    void EOR();	//exclusive OR
    void LDC();	//load coprocessor from memory
    void LDM();	//load multiple registers
    void LDR();	//load register from memory
    void LDRH();	//load halfword
    void LDRSB();	//load signed byte
    void LDRSH();	//load signed halfword
    void MCR();	//move cpu register to coprocessor register
    void MLA();	//multiply accumulative
    void MLAL();//multiply accumulative long
    void MOV();	//move register or constant
    void MRC();	//move coprocessor register to cpu register
    void MRS();	//move PRS status/flags to register
    void MSR();	//move register to PRS status/flags
    void MUL();	//multiply
    void MULL();//multiply long
    void MVN();	//move negative register
    void ORR();	//OR
    void RSB();	//reverse subtract
    void RSC();	//reverse subtract with carry
    void SBC();	//subtract with carry
    void STC();	//store coprocessor register to memory
    void STM();	//store multiple
    void STR();	//store register to memory
    void STRH();	//store halfword
    void SUB();	//subtract
    void SWI();	//software interrupt
    void SWP();	//swap register with memory
    void TEQ();	//test bitwiser equality
    void TST();	//test bits
    void UND();	//undefined instruction

    const InstrPointer ARM_table[257][16] = {
        {&ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::MUL, &ARMisa::AND, &ARMisa::STRH, &ARMisa::AND, &ARMisa::UND, &ARMisa::AND, &ARMisa::UND},
		{&ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::MUL, &ARMisa::AND, &ARMisa::LDRH, &ARMisa::AND, &ARMisa::LDRSB, &ARMisa::AND, &ARMisa::LDRSH},
		{&ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::MLA, &ARMisa::EOR, &ARMisa::STRH, &ARMisa::EOR, &ARMisa::UND, &ARMisa::EOR, &ARMisa::UND},
		{&ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::MLA, &ARMisa::EOR, &ARMisa::LDRH, &ARMisa::EOR, &ARMisa::LDRSB, &ARMisa::EOR, &ARMisa::LDRSH},
		{&ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::UND, &ARMisa::SUB, &ARMisa::STRH, &ARMisa::SUB, &ARMisa::UND, &ARMisa::SUB, &ARMisa::UND},
		{&ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::UND, &ARMisa::SUB, &ARMisa::LDRH, &ARMisa::SUB, &ARMisa::LDRSB, &ARMisa::SUB, &ARMisa::LDRSH},
		{&ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::UND, &ARMisa::RSB, &ARMisa::STRH, &ARMisa::RSB, &ARMisa::UND, &ARMisa::RSB, &ARMisa::UND},
		{&ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::UND, &ARMisa::RSB, &ARMisa::LDRH, &ARMisa::RSB, &ARMisa::LDRSB, &ARMisa::RSB, &ARMisa::LDRSH},
		{&ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::MULL, &ARMisa::ADD, &ARMisa::STRH, &ARMisa::ADD, &ARMisa::UND, &ARMisa::ADD, &ARMisa::UND},
		{&ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::MULL, &ARMisa::ADD, &ARMisa::LDRH, &ARMisa::ADD, &ARMisa::LDRSB, &ARMisa::ADD, &ARMisa::LDRSH},
		{&ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::MLAL, &ARMisa::ADC, &ARMisa::STRH, &ARMisa::ADC, &ARMisa::UND, &ARMisa::ADC, &ARMisa::UND},
		{&ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::MLAL, &ARMisa::ADC, &ARMisa::LDRH, &ARMisa::ADC, &ARMisa::LDRSB, &ARMisa::ADC, &ARMisa::LDRSH},
		{&ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::MULL, &ARMisa::SBC, &ARMisa::STRH, &ARMisa::SBC, &ARMisa::UND, &ARMisa::SBC, &ARMisa::UND},
		{&ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::MULL, &ARMisa::SBC, &ARMisa::LDRH, &ARMisa::SBC, &ARMisa::LDRSB, &ARMisa::SBC, &ARMisa::LDRSH},
		{&ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::MLAL, &ARMisa::RSC, &ARMisa::STRH, &ARMisa::RSC, &ARMisa::UND, &ARMisa::RSC, &ARMisa::UND},
		{&ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::MLAL, &ARMisa::RSC, &ARMisa::LDRH, &ARMisa::RSC, &ARMisa::LDRSB, &ARMisa::RSC, &ARMisa::LDRSH},
		{&ARMisa::MRS, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::TST, &ARMisa::UND, &ARMisa::UND, &ARMisa::TST, &ARMisa::SWP, &ARMisa::TST, &ARMisa::STRH, &ARMisa::TST, &ARMisa::UND, &ARMisa::TST, &ARMisa::UND},
		{&ARMisa::TST, &ARMisa::TST, &ARMisa::TST, &ARMisa::TST, &ARMisa::TST, &ARMisa::TST, &ARMisa::TST, &ARMisa::TST, &ARMisa::TST, &ARMisa::UND, &ARMisa::TST, &ARMisa::LDRH, &ARMisa::TST, &ARMisa::LDRSB, &ARMisa::TST, &ARMisa::LDRSH},
		{&ARMisa::MSR, &ARMisa::BX, &ARMisa::UND, &ARMisa::TEQ, &ARMisa::UND, &ARMisa::TEQ, &ARMisa::UND, &ARMisa::TEQ, &ARMisa::TEQ, &ARMisa::UND, &ARMisa::TEQ, &ARMisa::STRH, &ARMisa::TEQ, &ARMisa::UND, &ARMisa::TEQ, &ARMisa::UND},
		{&ARMisa::TEQ, &ARMisa::TEQ, &ARMisa::TEQ, &ARMisa::TEQ, &ARMisa::TEQ, &ARMisa::TEQ, &ARMisa::TEQ, &ARMisa::TEQ, &ARMisa::TEQ, &ARMisa::UND, &ARMisa::TEQ, &ARMisa::LDRH, &ARMisa::TEQ, &ARMisa::LDRSB, &ARMisa::TEQ, &ARMisa::LDRSH},
		{&ARMisa::MRS, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::CMP, &ARMisa::UND, &ARMisa::UND, &ARMisa::CMP, &ARMisa::SWP, &ARMisa::CMP, &ARMisa::STRH, &ARMisa::CMP, &ARMisa::UND, &ARMisa::CMP, &ARMisa::UND},
		{&ARMisa::CMP, &ARMisa::CMP, &ARMisa::CMP, &ARMisa::CMP, &ARMisa::CMP, &ARMisa::CMP, &ARMisa::CMP, &ARMisa::CMP, &ARMisa::CMP, &ARMisa::UND, &ARMisa::CMP, &ARMisa::LDRH, &ARMisa::CMP, &ARMisa::LDRSB, &ARMisa::CMP, &ARMisa::LDRSH},
		{&ARMisa::MSR, &ARMisa::CMN, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::CMN, &ARMisa::UND, &ARMisa::UND, &ARMisa::CMN, &ARMisa::UND, &ARMisa::CMN, &ARMisa::STRH, &ARMisa::CMN, &ARMisa::UND, &ARMisa::CMN, &ARMisa::UND},
		{&ARMisa::CMN, &ARMisa::CMN, &ARMisa::CMN, &ARMisa::CMN, &ARMisa::CMN, &ARMisa::CMN, &ARMisa::CMN, &ARMisa::CMN, &ARMisa::CMN, &ARMisa::UND, &ARMisa::CMN, &ARMisa::LDRH, &ARMisa::CMN, &ARMisa::LDRSB, &ARMisa::CMN, &ARMisa::LDRSH},
		{&ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::UND, &ARMisa::ORR, &ARMisa::STRH, &ARMisa::ORR, &ARMisa::UND, &ARMisa::ORR, &ARMisa::UND},
		{&ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::UND, &ARMisa::ORR, &ARMisa::LDRH, &ARMisa::ORR, &ARMisa::LDRSB, &ARMisa::ORR, &ARMisa::LDRSH},
		{&ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::UND, &ARMisa::MOV, &ARMisa::STRH, &ARMisa::MOV, &ARMisa::UND, &ARMisa::MOV, &ARMisa::UND},
		{&ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::UND, &ARMisa::MOV, &ARMisa::LDRH, &ARMisa::MOV, &ARMisa::LDRSB, &ARMisa::MOV, &ARMisa::LDRSH},
		{&ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::UND, &ARMisa::BIC, &ARMisa::STRH, &ARMisa::BIC, &ARMisa::UND, &ARMisa::BIC, &ARMisa::UND},
		{&ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::UND, &ARMisa::BIC, &ARMisa::LDRH, &ARMisa::BIC, &ARMisa::LDRSB, &ARMisa::BIC, &ARMisa::LDRSH},
		{&ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::UND, &ARMisa::MVN, &ARMisa::STRH, &ARMisa::MVN, &ARMisa::UND, &ARMisa::MVN, &ARMisa::UND},
		{&ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::UND, &ARMisa::MVN, &ARMisa::LDRH, &ARMisa::MVN, &ARMisa::LDRSB, &ARMisa::MVN, &ARMisa::LDRSH},
		{&ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND},
		{&ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND, &ARMisa::AND},
		{&ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR},
		{&ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR, &ARMisa::EOR},
		{&ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB},
		{&ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB, &ARMisa::SUB},
		{&ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB},
		{&ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB, &ARMisa::RSB},
		{&ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD},
		{&ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD, &ARMisa::ADD},
		{&ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC},
		{&ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC, &ARMisa::ADC},
		{&ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC},
		{&ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC, &ARMisa::SBC},
		{&ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC},
		{&ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC, &ARMisa::RSC},
		{&ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND},
		{&ARMisa::TST, &ARMisa::TST, &ARMisa::TST, &ARMisa::TST, &ARMisa::TST, &ARMisa::TST, &ARMisa::TST, &ARMisa::TST, &ARMisa::TST, &ARMisa::TST, &ARMisa::TST, &ARMisa::TST, &ARMisa::TST, &ARMisa::TST, &ARMisa::TST, &ARMisa::TST},
		{&ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR},
		{&ARMisa::TEQ, &ARMisa::TEQ, &ARMisa::TEQ, &ARMisa::TEQ, &ARMisa::TEQ, &ARMisa::TEQ, &ARMisa::TEQ, &ARMisa::TEQ, &ARMisa::TEQ, &ARMisa::TEQ, &ARMisa::TEQ, &ARMisa::TEQ, &ARMisa::TEQ, &ARMisa::TEQ, &ARMisa::TEQ, &ARMisa::TEQ},
		{&ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND, &ARMisa::UND},
		{&ARMisa::CMP, &ARMisa::CMP, &ARMisa::CMP, &ARMisa::CMP, &ARMisa::CMP, &ARMisa::CMP, &ARMisa::CMP, &ARMisa::CMP, &ARMisa::CMP, &ARMisa::CMP, &ARMisa::CMP, &ARMisa::CMP, &ARMisa::CMP, &ARMisa::CMP, &ARMisa::CMP, &ARMisa::CMP},
		{&ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR, &ARMisa::MSR},
		{&ARMisa::CMN, &ARMisa::CMN, &ARMisa::CMN, &ARMisa::CMN, &ARMisa::CMN, &ARMisa::CMN, &ARMisa::CMN, &ARMisa::CMN, &ARMisa::CMN, &ARMisa::CMN, &ARMisa::CMN, &ARMisa::CMN, &ARMisa::CMN, &ARMisa::CMN, &ARMisa::CMN, &ARMisa::CMN},
		{&ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR},
		{&ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR, &ARMisa::ORR},
		{&ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV},
		{&ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV, &ARMisa::MOV},
		{&ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC},
		{&ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC, &ARMisa::BIC},
		{&ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN},
		{&ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN, &ARMisa::MVN},
		{&ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR},
		{&ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR},
		{&ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR},
		{&ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR},
		{&ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR},
		{&ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR},
		{&ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR},
		{&ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR},
		{&ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR},
		{&ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR},
		{&ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR},
		{&ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR},
		{&ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR},
		{&ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR},
		{&ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR},
		{&ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR},
		{&ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR},
		{&ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR},
		{&ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR},
		{&ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR},
		{&ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR},
		{&ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR},
		{&ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR},
		{&ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR},
		{&ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR},
		{&ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR},
		{&ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR},
		{&ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR},
		{&ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR},
		{&ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR},
		{&ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR, &ARMisa::STR},
		{&ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR, &ARMisa::LDR},
		{&ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND},
		{&ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND},
		{&ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND},
		{&ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND},
		{&ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND},
		{&ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND},
		{&ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND},
		{&ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND},
		{&ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND},
		{&ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND},
		{&ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND},
		{&ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND},
		{&ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND},
		{&ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND},
		{&ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND},
		{&ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND},
		{&ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND},
		{&ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND},
		{&ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND},
		{&ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND},
		{&ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND},
		{&ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND},
		{&ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND},
		{&ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND},
		{&ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND},
		{&ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND},
		{&ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND},
		{&ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND},
		{&ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND},
		{&ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND},
		{&ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND, &ARMisa::STR, &ARMisa::UND},
		{&ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND, &ARMisa::LDR, &ARMisa::UND},
		{&ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM},
		{&ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM},
		{&ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM},
		{&ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM},
		{&ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM},
		{&ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM},
		{&ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM},
		{&ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM},
		{&ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM},
		{&ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM},
		{&ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM},
		{&ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM},
		{&ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM},
		{&ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM},
		{&ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM},
		{&ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM},
		{&ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM},
		{&ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM},
		{&ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM},
		{&ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM},
		{&ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM},
		{&ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM},
		{&ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM},
		{&ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM},
		{&ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM},
		{&ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM},
		{&ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM},
		{&ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM},
		{&ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM},
		{&ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM},
		{&ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM, &ARMisa::STM},
		{&ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM, &ARMisa::LDM},
		{&ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B},
		{&ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B},
		{&ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B},
		{&ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B},
		{&ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B},
		{&ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B},
		{&ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B},
		{&ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B},
		{&ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B},
		{&ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B},
		{&ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B},
		{&ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B},
		{&ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B},
		{&ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B},
		{&ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B},
		{&ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B, &ARMisa::B},
		{&ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL},
		{&ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL},
		{&ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL},
		{&ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL},
		{&ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL},
		{&ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL},
		{&ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL},
		{&ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL},
		{&ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL},
		{&ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL},
		{&ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL},
		{&ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL},
		{&ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL},
		{&ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL},
		{&ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL},
		{&ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL, &ARMisa::BL},
		{&ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC},
		{&ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC},
		{&ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC},
		{&ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC},
		{&ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC},
		{&ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC},
		{&ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC},
		{&ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC},
		{&ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC},
		{&ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC},
		{&ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC},
		{&ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC},
		{&ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC},
		{&ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC},
		{&ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC},
		{&ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC},
		{&ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC},
		{&ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC},
		{&ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC},
		{&ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC},
		{&ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC},
		{&ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC},
		{&ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC},
		{&ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC},
		{&ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC},
		{&ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC},
		{&ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC},
		{&ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC},
		{&ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC},
		{&ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC},
		{&ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC, &ARMisa::STC},
		{&ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC, &ARMisa::LDC},
		{&ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR},
		{&ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC},
		{&ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR},
		{&ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC},
		{&ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR},
		{&ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC},
		{&ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR},
		{&ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC},
		{&ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR},
		{&ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC},
		{&ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR},
		{&ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC},
		{&ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR},
		{&ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC},
		{&ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR, &ARMisa::CDP, &ARMisa::MCR},
		{&ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC, &ARMisa::CDP, &ARMisa::MRC},
		{&ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI},
		{&ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI},
		{&ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI},
		{&ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI},
		{&ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI},
		{&ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI},
		{&ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI},
		{&ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI},
		{&ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI},
		{&ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI},
		{&ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI},
		{&ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI},
		{&ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI},
		{&ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI},
		{&ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI},
		{&ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI, &ARMisa::SWI}
	};

};

#endif //UARM_ARMISA_H
