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
	
	void passCoprocessors(coprocessor **copList) {cpint->init(copList);};
	coprocessor_interface *getCopInt() {return cpint;};
	
	void setEndianess(bool bigEndian) {BIGEND_sig = bigEndian;};	//system is set little endian by default, use this method to change the setting
	ProcessorStatus getStatus() {return status;};
	Word *getRegister(Byte reg) {return &cpu_registers[reg];};
	
	Word *getVisibleRegister(Byte reg);
	
	systemBus *getBus() {return bus;};
private:
	coprocessor_interface *cpint;
	ProcessorStatus status;
	Word cpu_registers[CPU_REGISTERS_NUM];
	Word shifter_operand, alu_tmp;
	bool shifter_carry_out;
	bool BIGEND_sig;
	
	ProcessorMode getMode() {uint16_t mode = cpu_registers[REG_CPSR] & MODE_MASK; return (ProcessorMode) mode;};
	bool condCheck();
	void barrelShifter(bool immediate, Byte byte, Byte half);
	
	void execute();
	void dataProcessing();
	void multiply();
	void singleDataSwap();
	void halfwordDataTransfer();
	void branch();
	void coprocessorInstr();
	void singleDataTransfer();
	void blockDataTransfer();
	void undefined();
	void NOP() {};
	void unpredictable();
	
	
	void ADC(Word op1, Word op2, Word *dest);	//add with carry
	void ADD(Word op1, Word op2, Word *dest);	//add
	void AND(Word op1, Word op2, Word *dest);	//AND
	void B();	//brach
	void BIC(Word op1, Word op2, Word *dest);	//bit clear
	void BL();	//branch with link
	void BX();	//branch and exchange
	void CDP();	//coprocessor data processing
	void CMN(Word op1, Word op2, Word *dest); //compare negative
	void CMP(Word op1, Word op2, Word *dest); //compare
	void EOR(Word op1, Word op2, Word *dest);	//exclusive OR
	void LDC();	//load coprocessor from memory
	void LDM();	//load multiple registers
	void LDR(bool P, bool U, bool B, bool W, Word *src, Word offset, Word *dest);	//load register from memory
	void MCR();	//move cpu register to coprocessor register
	void MLA();	//multiply accumulative
	void MOV(Word op1, Word op2, Word *dest);	//move register or constant
	void MRC();	//move coprocessor register to cpu register
	void MRS();	//move PRS status/flags to register
	void MSR();	//move register to PRS status/flags
	void MUL();	//multiply
	void MVN(Word op1, Word op2, Word *dest);	//move negative register
	void ORR(Word op1, Word op2, Word *dest);	//OR
	void RSB(Word op1, Word op2, Word *dest);	//reverse subtract
	void RSC(Word op1, Word op2, Word *dest);	//reverse subtract with carry
	void SBC(Word op1, Word op2, Word *dest);	//subtract with carry
	void STC();	//store coprocessor register to memory
	void STM();	//store multiple
	void STR(bool P, bool U, bool B, bool W, Word *src, Word offset, Word *dest);	//store register to memory
	void SUB(Word op1, Word op2, Word *dest);	//subtract
	void SWI();	//software interrupt
	void SWP();	//swap register with memory
	void TEQ(Word op1, Word op2, Word *dest);	//test bitwiser equality
	void TST(Word op1, Word op2, Word *dest);	//test bits
	
	void singleMemoryAccess(bool L, bool P, bool U, bool B, bool W, Word *src, Word offset, Word *dest);
	void dataPsum(Word op1, Word op2, bool carry, bool sum, Word *dest);
	void bitwiseReturn(Word *dest);
};

#endif //UARM_PROCESSOR_CC
