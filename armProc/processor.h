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

#ifndef UARM_PROCESSOR_H
#define UARM_PROCESSOR_H

#include "pu.h"
#include "coprocessor_interface.h"

enum ProcessorStatus {
    PS_HALTED,
    PS_RUNNING,
    PS_IDLE
};

enum ProcessorMode {	//new modes MUST be added to switch in setStatusRegister implementation
	MODE_USER = 0x10,
	MODE_FAST_INTERRUPT = 0x11,
	MODE_INTERRUPT = 0x12,
	MODE_SUPERVISOR = 0x13,
	MODE_ABORT = 0x17,
	MODE_UNDEFINED = 0x1B,
	MODE_SYSTEM = 0x1F,
};

enum ExceptionMode {	//values correspond to fixed low exception vector addresses
	EXC_RESET	= 0x00000000,
	EXC_UNDEF	= 0x00000004,
	EXC_SWI		= 0x00000008,
	EXC_PREFABT = 0x0000000C,
	EXC_DATAABT = 0x00000010,
	EXC_IRQ		= 0x00000018,
	EXC_FIQ		= 0x0000001C,
};

class processor : public pu{
public:
	processor();
	~processor() {delete cpint;};
	
	Word *getPC() {return &cpu_registers[REG_PC];};
	void nextCycle();
	
	void prefetch();
	
	coprocessor_interface *getCopInt() {return cpint;};
	
	void setEndianess(bool bigEndian) {BIGEND_sig = bigEndian;};	//system is set little endian by default, use this method to change the setting
	ProcessorStatus getStatus() {return status;};
	Word *getRegister(Byte reg) {return &cpu_registers[reg];};
	
	Word *getVisibleRegister(Byte reg);
	
	Word *getPipeline(unsigned int i) {return &pipeline[i];};
	
	systemBus *getBus() {return bus;};
	
	/* processor could abort the execution cycle of coprocessors in case of interrupts or traps
	 * also, the first two cycles only do fetches to load the pipeline
	 */
	void cycle() {
		fetch();
		decode();
		setOP("Unknown");
		execute();
	};
	
	Word OPcode;
	bool isOPcodeARM = true;
	string mnemonicOPcode;
private:
	coprocessor_interface *cpint;
	ProcessorStatus status;
	Word pipeline[PIPELINE_STAGES];
	Word cpu_registers[CPU_REGISTERS_NUM];
	Word shifter_operand, alu_tmp;
	bool shifter_carry_out;
	bool BIGEND_sig;
	
	ProcessorMode getMode() {uint16_t mode = cpu_registers[REG_CPSR] & MODE_MASK; return (ProcessorMode) mode;};
	void debugARM(string mnemonic);
	bool condCheck();
	void barrelShifter(bool immediate, Byte byte, Byte half);
	
	void undefinedTrap();
	void softwareInterruptTrap();
	void execTrap(ExceptionMode exception);
	void NOP() {debugARM("NOP");};
	void unpredictable();
	Word get_unpredictable();
	bool get_unpredictableB();
	
	void fetch() {pipeline[PIPELINE_EXECUTE] = bus->pipeline[PIPELINE_EXECUTE]; pipeline[PIPELINE_DECODE] = bus->pipeline[PIPELINE_DECODE]; pipeline[PIPELINE_FETCH] = bus->pipeline[PIPELINE_FETCH];};
	void decode() {};
	void execute();
	
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
	
	
	void multiply(bool accumulate, bool lngWord);
	void coprocessorTransfer(bool memAcc, bool toCoproc);
	void coprocessorOperation();
	void singleDataSwap();
	void blockDataTransfer(bool load);
	void accessPSR(bool load);
	void branch(bool link, bool exchange);
	void dataProcessing(Byte opcode);
	void halfwordDataTransfer(bool sign, bool load_halfwd);
	void singleMemoryAccess(bool L);
	void dataPsum(Word op1, Word op2, bool carry, bool sum, Word *dest);
	void bitwiseReturn(Word *dest);
	
	void loadStore(bool load, bool P, bool U, bool B, bool W, Word* srcDst, Word* base, Word offset);
	
	typedef void(processor::*InstrPointer)();
	
	const InstrPointer ARM_table[257][16] = {
		{&processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::MUL, &processor::AND, &processor::STRH, &processor::AND, &processor::UND, &processor::AND, &processor::UND},
		{&processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::MUL, &processor::AND, &processor::LDRH, &processor::AND, &processor::LDRSB, &processor::AND, &processor::LDRSH},
		{&processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::MLA, &processor::EOR, &processor::STRH, &processor::EOR, &processor::UND, &processor::EOR, &processor::UND},
		{&processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::MLA, &processor::EOR, &processor::LDRH, &processor::EOR, &processor::LDRSB, &processor::EOR, &processor::LDRSH},
		{&processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::UND, &processor::SUB, &processor::STRH, &processor::SUB, &processor::UND, &processor::SUB, &processor::UND},
		{&processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::UND, &processor::SUB, &processor::LDRH, &processor::SUB, &processor::LDRSB, &processor::SUB, &processor::LDRSH},
		{&processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::UND, &processor::RSB, &processor::STRH, &processor::RSB, &processor::UND, &processor::RSB, &processor::UND},
		{&processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::UND, &processor::RSB, &processor::LDRH, &processor::RSB, &processor::LDRSB, &processor::RSB, &processor::LDRSH},
		{&processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::MULL, &processor::ADD, &processor::STRH, &processor::ADD, &processor::UND, &processor::ADD, &processor::UND},
		{&processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::MULL, &processor::ADD, &processor::LDRH, &processor::ADD, &processor::LDRSB, &processor::ADD, &processor::LDRSH},
		{&processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::MLAL, &processor::ADC, &processor::STRH, &processor::ADC, &processor::UND, &processor::ADC, &processor::UND},
		{&processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::MLAL, &processor::ADC, &processor::LDRH, &processor::ADC, &processor::LDRSB, &processor::ADC, &processor::LDRSH},
		{&processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::MULL, &processor::SBC, &processor::STRH, &processor::SBC, &processor::UND, &processor::SBC, &processor::UND},
		{&processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::MULL, &processor::SBC, &processor::LDRH, &processor::SBC, &processor::LDRSB, &processor::SBC, &processor::LDRSH},
		{&processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::MLAL, &processor::RSC, &processor::STRH, &processor::RSC, &processor::UND, &processor::RSC, &processor::UND},
		{&processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::MLAL, &processor::RSC, &processor::LDRH, &processor::RSC, &processor::LDRSB, &processor::RSC, &processor::LDRSH},
		{&processor::MRS, &processor::UND, &processor::UND, &processor::UND, &processor::UND, &processor::TST, &processor::UND, &processor::UND, &processor::TST, &processor::SWP, &processor::TST, &processor::STRH, &processor::TST, &processor::UND, &processor::TST, &processor::UND},
		{&processor::TST, &processor::TST, &processor::TST, &processor::TST, &processor::TST, &processor::TST, &processor::TST, &processor::TST, &processor::TST, &processor::UND, &processor::TST, &processor::LDRH, &processor::TST, &processor::LDRSB, &processor::TST, &processor::LDRSH},
		{&processor::MSR, &processor::BX, &processor::UND, &processor::TEQ, &processor::UND, &processor::TEQ, &processor::UND, &processor::TEQ, &processor::TEQ, &processor::UND, &processor::TEQ, &processor::STRH, &processor::TEQ, &processor::UND, &processor::TEQ, &processor::UND},
		{&processor::TEQ, &processor::TEQ, &processor::TEQ, &processor::TEQ, &processor::TEQ, &processor::TEQ, &processor::TEQ, &processor::TEQ, &processor::TEQ, &processor::UND, &processor::TEQ, &processor::LDRH, &processor::TEQ, &processor::LDRSB, &processor::TEQ, &processor::LDRSH},
		{&processor::MRS, &processor::UND, &processor::UND, &processor::UND, &processor::UND, &processor::CMP, &processor::UND, &processor::UND, &processor::CMP, &processor::SWP, &processor::CMP, &processor::STRH, &processor::CMP, &processor::UND, &processor::CMP, &processor::UND},
		{&processor::CMP, &processor::CMP, &processor::CMP, &processor::CMP, &processor::CMP, &processor::CMP, &processor::CMP, &processor::CMP, &processor::CMP, &processor::UND, &processor::CMP, &processor::LDRH, &processor::CMP, &processor::LDRSB, &processor::CMP, &processor::LDRSH},
		{&processor::MSR, &processor::CMN, &processor::UND, &processor::UND, &processor::UND, &processor::CMN, &processor::UND, &processor::UND, &processor::CMN, &processor::UND, &processor::CMN, &processor::STRH, &processor::CMN, &processor::UND, &processor::CMN, &processor::UND},
		{&processor::CMN, &processor::CMN, &processor::CMN, &processor::CMN, &processor::CMN, &processor::CMN, &processor::CMN, &processor::CMN, &processor::CMN, &processor::UND, &processor::CMN, &processor::LDRH, &processor::CMN, &processor::LDRSB, &processor::CMN, &processor::LDRSH},
		{&processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::UND, &processor::ORR, &processor::STRH, &processor::ORR, &processor::UND, &processor::ORR, &processor::UND},
		{&processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::UND, &processor::ORR, &processor::LDRH, &processor::ORR, &processor::LDRSB, &processor::ORR, &processor::LDRSH},
		{&processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::UND, &processor::MOV, &processor::STRH, &processor::MOV, &processor::UND, &processor::MOV, &processor::UND},
		{&processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::UND, &processor::MOV, &processor::LDRH, &processor::MOV, &processor::LDRSB, &processor::MOV, &processor::LDRSH},
		{&processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::UND, &processor::BIC, &processor::STRH, &processor::BIC, &processor::UND, &processor::BIC, &processor::UND},
		{&processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::UND, &processor::BIC, &processor::LDRH, &processor::BIC, &processor::LDRSB, &processor::BIC, &processor::LDRSH},
		{&processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::UND, &processor::MVN, &processor::STRH, &processor::MVN, &processor::UND, &processor::MVN, &processor::UND},
		{&processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::UND, &processor::MVN, &processor::LDRH, &processor::MVN, &processor::LDRSB, &processor::MVN, &processor::LDRSH},
		{&processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND},
		{&processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND, &processor::AND},
		{&processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR},
		{&processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR, &processor::EOR},
		{&processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB},
		{&processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB, &processor::SUB},
		{&processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB},
		{&processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB, &processor::RSB},
		{&processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD},
		{&processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD, &processor::ADD},
		{&processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC},
		{&processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC, &processor::ADC},
		{&processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC},
		{&processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC, &processor::SBC},
		{&processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC},
		{&processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC, &processor::RSC},
		{&processor::UND, &processor::UND, &processor::UND, &processor::UND, &processor::UND, &processor::UND, &processor::UND, &processor::UND, &processor::UND, &processor::UND, &processor::UND, &processor::UND, &processor::UND, &processor::UND, &processor::UND, &processor::UND},
		{&processor::TST, &processor::TST, &processor::TST, &processor::TST, &processor::TST, &processor::TST, &processor::TST, &processor::TST, &processor::TST, &processor::TST, &processor::TST, &processor::TST, &processor::TST, &processor::TST, &processor::TST, &processor::TST},
		{&processor::MSR, &processor::MSR, &processor::MSR, &processor::MSR, &processor::MSR, &processor::MSR, &processor::MSR, &processor::MSR, &processor::MSR, &processor::MSR, &processor::MSR, &processor::MSR, &processor::MSR, &processor::MSR, &processor::MSR, &processor::MSR},
		{&processor::TEQ, &processor::TEQ, &processor::TEQ, &processor::TEQ, &processor::TEQ, &processor::TEQ, &processor::TEQ, &processor::TEQ, &processor::TEQ, &processor::TEQ, &processor::TEQ, &processor::TEQ, &processor::TEQ, &processor::TEQ, &processor::TEQ, &processor::TEQ},
		{&processor::UND, &processor::UND, &processor::UND, &processor::UND, &processor::UND, &processor::UND, &processor::UND, &processor::UND, &processor::UND, &processor::UND, &processor::UND, &processor::UND, &processor::UND, &processor::UND, &processor::UND, &processor::UND},
		{&processor::CMP, &processor::CMP, &processor::CMP, &processor::CMP, &processor::CMP, &processor::CMP, &processor::CMP, &processor::CMP, &processor::CMP, &processor::CMP, &processor::CMP, &processor::CMP, &processor::CMP, &processor::CMP, &processor::CMP, &processor::CMP},
		{&processor::MSR, &processor::MSR, &processor::MSR, &processor::MSR, &processor::MSR, &processor::MSR, &processor::MSR, &processor::MSR, &processor::MSR, &processor::MSR, &processor::MSR, &processor::MSR, &processor::MSR, &processor::MSR, &processor::MSR, &processor::MSR},
		{&processor::CMN, &processor::CMN, &processor::CMN, &processor::CMN, &processor::CMN, &processor::CMN, &processor::CMN, &processor::CMN, &processor::CMN, &processor::CMN, &processor::CMN, &processor::CMN, &processor::CMN, &processor::CMN, &processor::CMN, &processor::CMN},
		{&processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR},
		{&processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR, &processor::ORR},
		{&processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV},
		{&processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV, &processor::MOV},
		{&processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC},
		{&processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC, &processor::BIC},
		{&processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN},
		{&processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN, &processor::MVN},
		{&processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR},
		{&processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR},
		{&processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR},
		{&processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR},
		{&processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR},
		{&processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR},
		{&processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR},
		{&processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR},
		{&processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR},
		{&processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR},
		{&processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR},
		{&processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR},
		{&processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR},
		{&processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR},
		{&processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR},
		{&processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR},
		{&processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR},
		{&processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR},
		{&processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR},
		{&processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR},
		{&processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR},
		{&processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR},
		{&processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR},
		{&processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR},
		{&processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR},
		{&processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR},
		{&processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR},
		{&processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR},
		{&processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR},
		{&processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR},
		{&processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR, &processor::STR},
		{&processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR, &processor::LDR},
		{&processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND},
		{&processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND},
		{&processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND},
		{&processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND},
		{&processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND},
		{&processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND},
		{&processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND},
		{&processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND},
		{&processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND},
		{&processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND},
		{&processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND},
		{&processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND},
		{&processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND},
		{&processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND},
		{&processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND},
		{&processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND},
		{&processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND},
		{&processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND},
		{&processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND},
		{&processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND},
		{&processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND},
		{&processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND},
		{&processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND},
		{&processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND},
		{&processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND},
		{&processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND},
		{&processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND},
		{&processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND},
		{&processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND},
		{&processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND},
		{&processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND, &processor::STR, &processor::UND},
		{&processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND, &processor::LDR, &processor::UND},
		{&processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM},
		{&processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM},
		{&processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM},
		{&processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM},
		{&processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM},
		{&processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM},
		{&processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM},
		{&processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM},
		{&processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM},
		{&processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM},
		{&processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM},
		{&processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM},
		{&processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM},
		{&processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM},
		{&processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM},
		{&processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM},
		{&processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM},
		{&processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM},
		{&processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM},
		{&processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM},
		{&processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM},
		{&processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM},
		{&processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM},
		{&processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM},
		{&processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM},
		{&processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM},
		{&processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM},
		{&processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM},
		{&processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM},
		{&processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM},
		{&processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM, &processor::STM},
		{&processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM, &processor::LDM},
		{&processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B},
		{&processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B},
		{&processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B},
		{&processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B},
		{&processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B},
		{&processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B},
		{&processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B},
		{&processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B},
		{&processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B},
		{&processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B},
		{&processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B},
		{&processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B},
		{&processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B},
		{&processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B},
		{&processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B},
		{&processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B, &processor::B},
		{&processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL},
		{&processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL},
		{&processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL},
		{&processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL},
		{&processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL},
		{&processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL},
		{&processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL},
		{&processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL},
		{&processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL},
		{&processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL},
		{&processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL},
		{&processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL},
		{&processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL},
		{&processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL},
		{&processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL},
		{&processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL, &processor::BL},
		{&processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC},
		{&processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC},
		{&processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC},
		{&processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC},
		{&processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC},
		{&processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC},
		{&processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC},
		{&processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC},
		{&processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC},
		{&processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC},
		{&processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC},
		{&processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC},
		{&processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC},
		{&processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC},
		{&processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC},
		{&processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC},
		{&processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC},
		{&processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC},
		{&processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC},
		{&processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC},
		{&processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC},
		{&processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC},
		{&processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC},
		{&processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC},
		{&processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC},
		{&processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC},
		{&processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC},
		{&processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC},
		{&processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC},
		{&processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC},
		{&processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC, &processor::STC},
		{&processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC, &processor::LDC},
		{&processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR},
		{&processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC},
		{&processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR},
		{&processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC},
		{&processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR},
		{&processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC},
		{&processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR},
		{&processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC},
		{&processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR},
		{&processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC},
		{&processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR},
		{&processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC},
		{&processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR},
		{&processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC},
		{&processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR, &processor::CDP, &processor::MCR},
		{&processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC, &processor::CDP, &processor::MRC},
		{&processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI},
		{&processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI},
		{&processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI},
		{&processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI},
		{&processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI},
		{&processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI},
		{&processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI},
		{&processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI},
		{&processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI},
		{&processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI},
		{&processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI},
		{&processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI},
		{&processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI},
		{&processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI},
		{&processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI},
		{&processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI, &processor::SWI}
	};
	
	void setOP(string mnemonic){
		if(true) {	//thumb support not yet implemented
			isOPcodeARM = true;
			OPcode = pipeline[PIPELINE_EXECUTE];
		} else {
			isOPcodeARM = false;
			OPcode = (HalfWord) pipeline[PIPELINE_EXECUTE] & 0xFFFF;	//this is only the first halfword case
		}
		mnemonicOPcode = mnemonic;
	};
	
};

#endif //UARM_PROCESSOR_CC
