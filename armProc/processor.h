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
    MODE_SYSTEM = 0x1F
};

enum ExceptionMode {	//values correspond to fixed low exception vector addresses
	EXC_RESET	= 0x00000000,
	EXC_UNDEF	= 0x00000004,
	EXC_SWI		= 0x00000008,
	EXC_PREFABT = 0x0000000C,
	EXC_DATAABT = 0x00000010,
	EXC_IRQ		= 0x00000018,
    EXC_FIQ		= 0x0000001C
};

class ARMisa;
class Thumbisa;

class processor : public pu{
public:
	processor();
    ~processor();
	
    Word *getPC() {return &cpu_registers[REG_PC];}
	void nextCycle();
	
	void prefetch();
	
    coprocessor_interface *getCopInt() {return cpint;}
	
    void setEndianess(bool bigEndian) {BIGEND_sig = bigEndian;}	//system is set little endian by default, use this method to change the setting
    ProcessorStatus getStatus() {return status;}
    Word *getRegister(Byte reg) {return &cpu_registers[reg];}
	
    Word *getRegList(){return cpu_registers;}

    Word *getVisibleRegister(Byte reg);
	
    Word *getPipeline(unsigned int i) {return &pipeline[i];}
	
    systemBus *getBus() {return bus;}
	
	/* processor could abort the execution cycle of coprocessors in case of interrupts or traps
	 * also, the first two cycles only do fetches to load the pipeline
	 */
	void cycle() {
		fetch();
		decode();
		setOP("Unknown", true);
		execute();
    }
	
	Word OPcode;
	bool isOPcodeARM = true;
	string mnemonicOPcode;
	friend class ARMisa;
    friend class Thumbisa;
private:
	coprocessor_interface *cpint;
	ProcessorStatus status;
	Word pipeline[PIPELINE_STAGES];
	Word cpu_registers[CPU_REGISTERS_NUM];
	Word shifter_operand, alu_tmp;
	bool shifter_carry_out;
	bool BIGEND_sig;
	
	ARMisa *execARM;
	Thumbisa *execThumb;
	
    ProcessorMode getMode() {uint16_t mode = cpu_registers[REG_CPSR] & MODE_MASK; return (ProcessorMode) mode;}
	void debugARM(string mnemonic);
	void debugThumb(string mnemonic);
	bool condCheck();
	void barrelShifter(bool immediate, Byte byte, Byte half);
	
	void undefinedTrap();
	void softwareInterruptTrap();
	void dataAbortTrap();
	void resetTrap();
	void prefetchAbortTrap();
	void interruptTrap();
	void fastInterruptTrap();
	void execTrap(ExceptionMode exception);
    void NOP() {debugARM("NOP");}
	void unpredictable();
	Word get_unpredictable();
	bool get_unpredictableB();
	
    void fetch() {pipeline[PIPELINE_EXECUTE] = bus->pipeline[PIPELINE_EXECUTE]; pipeline[PIPELINE_DECODE] = bus->pipeline[PIPELINE_DECODE]; pipeline[PIPELINE_FETCH] = bus->pipeline[PIPELINE_FETCH];}
    void decode() {}
	void execute();
	
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
	void dataPsum(Word op1, Word op2, bool carry, bool sum, Word *dest, bool S);
	void bitwiseReturn(Word *dest, bool S);
	
	void loadStore(bool load, bool P, bool U, bool B, bool W, Word* srcDst, Word* base, Word offset);

	void setOP(string mnemonic, bool isARM){
		if(isARM)
			OPcode = pipeline[PIPELINE_EXECUTE];
		else{
			if((*getPC() >> 1) & 1)	//second halfword
				OPcode = (HalfWord) (pipeline[PIPELINE_EXECUTE] >> 16) & 0xFFFF;
			else 					//first halfword
				OPcode = (HalfWord) pipeline[PIPELINE_EXECUTE] & 0xFFFF;	//this is only the first halfword case
		}
		isOPcodeARM = isARM;
		mnemonicOPcode = mnemonic;
    }
};

#endif //UARM_PROCESSOR_CC
