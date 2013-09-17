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

#ifndef UARM_PROCESSOR_CC
#define UARM_PROCESSOR_CC

#include "processor.h"
#include "services/util.h"
#include "ARMisa.h"
#include "Thumbisa.h"

processor::processor() : pu() {
	status = PS_IDLE;
	BIGEND_sig = ENDIANESS_BIGENDIAN;
	cpint = new coprocessor_interface();
	for(int i = 0; i < PIPELINE_STAGES; i++)
		pipeline[i] = 0;
    for(int i = 0; i < CPU_REGISTERS_NUM; i++)
        cpu_registers[i] = 0;
	cpu_registers[REG_CPSR] = MODE_USER;
	execARM = new ARMisa(this);
	execThumb = new Thumbisa(this);
}

processor::~processor(){
    delete cpint;
    delete execARM;
    delete execThumb;
}

/* ******************** *
 * 						*
 * Processor facilities *
 * 						*
 * ******************** */

Word *processor::getVisibleRegister(Byte num){
	if(num > 17)	//invalid register
		return NULL;
	if(num == 16)	//status register
		return &cpu_registers[REG_CPSR];
	if(num == 15)	//pc
		return &cpu_registers[REG_PC];
	if(num <= 7)	//common to all modes
		return &cpu_registers[num];
	
	ProcessorMode mode = getMode();
	if(mode != MODE_FAST_INTERRUPT && num < 13) //only fiq has special registers r7-r12
		return &cpu_registers[num];
	switch(mode){
		case MODE_USER:
		case MODE_SYSTEM:
			if(num > 16)	//system & user don't have any saved status register
				return NULL;
			return &cpu_registers[num];
			break;
		case MODE_FAST_INTERRUPT:
			if(num == 17)
				return &cpu_registers[REG_FIQ_BASE+7];
			return &cpu_registers[REG_FIQ_BASE+(num-8)];
			break;
		case MODE_INTERRUPT:
			if(num == 17)
				return &cpu_registers[REG_IRQ_BASE+2];
			return &cpu_registers[REG_IRQ_BASE+(num-13)];
			break;
		case MODE_SUPERVISOR:
			if(num == 17)
				return &cpu_registers[REG_SVC_BASE+2];
			return &cpu_registers[REG_SVC_BASE+(num-13)];
			break;
		case MODE_ABORT:
			if(num == 17)
				return &cpu_registers[REG_ABT_BASE+2];
			return &cpu_registers[REG_ABT_BASE+(num-13)];
			break;
		case MODE_UNDEFINED:
			if(num == 17)
				return &cpu_registers[REG_UNDEF_BASE+2];
			return &cpu_registers[REG_UNDEF_BASE+(num-13)];
			break;
		default:
			return NULL;
			break;
	}
	return NULL;
}

void processor::barrelShifter(bool immediate, Byte byte, Byte half){
	Byte amount;
	Word val;
	if(immediate){	//immediate shift: byte rotated right by 2*half
		amount = half*2;
		val = byte;
		if(amount != 0){
			shifter_operand = (val >> amount) | (val << (sizeof(Word)*8 - amount));
			shifter_carry_out = ((shifter_operand >> 31) == 1 ? true : false);
		} else {
			shifter_operand = val;
			shifter_carry_out = ((cpu_registers[REG_CPSR] & C_MASK) == 1 ? true : false);
		}
	} else {	//register shift, half is regnum byte is shift operand
		if((byte & 1) == 0){	//immediate amount
			amount = byte >> 3;
		} else {				//register amount
			if((byte >> 4) != 15){	//valid register
				amount = (Byte) ((*getVisibleRegister(byte >> 4)) & 0xFF);
			} else {				//r15 cannot be used
				amount = 0;
			}
		}
		val = *getVisibleRegister(half);
		switch((byte >> 1)&3){
			case 0:	//logical shift left
				if(amount == 0){
					shifter_operand = val;
					shifter_carry_out = ((cpu_registers[REG_CPSR] & C_MASK) == 1 ? true : false);
				} else {
					shifter_operand = val << amount;
					shifter_carry_out = ((val & (1<<(32-amount))) == 1 ? true : false);
				}
				break;
			case 1:	//logical shift right
				if(amount == 0){	//special case, represents LSR #32
					shifter_operand = 0;
					shifter_carry_out = ((val >> 31) == 1 ? true : false);
				} else {
					shifter_operand = val >> amount;
					shifter_carry_out = ((val & 1<<(amount-1)) == 1 ? true : false);
				}
				break;
			case 2:	//arithmetic shift right
				if(amount == 0){	//special case, represents ASR #32
					shifter_operand = ((val >> 31) == 1 ? 0xFFFFFFFF : 0);
					shifter_carry_out = ((val >> 31) == 1 ? true : false);
				} else {
					Word ret = val >> amount;
					if((val >> 31) == 1)
						ret |= 0xFFFFFFFF << (32 - amount);
					else
						ret &= 0xFFFFFFFF >> (31 - amount);
					shifter_operand = ret;
					shifter_carry_out = ((val & 1<<(amount-1)) == 1 ? true : false);
				}
				break;
			case 3:	//rotate right
				if(amount == 0){ //special case: rotate right extended
					shifter_operand = val >> 1 | ((cpu_registers[REG_CPSR] & C_MASK) << 31);
					shifter_carry_out = ((val & 1) == 1 ? true : false);
				} else {
					shifter_operand = (val >> amount) | (val << (sizeof(Word)*8 - amount));
					shifter_carry_out = ((shifter_operand >> 31) == 1 ? true : false);
				}
				break;
		}
	}
}

bool processor::condCheck(){
	Byte cond = pipeline[PIPELINE_EXECUTE] >> 28;
	switch(cond){
		case 0:	//EQ
			if(util::getInstance()->checkBit(&cpu_registers[REG_CPSR], Z_POS))
				return true;
			else
				return false;
			break;
		case 1:	//NE
			if(util::getInstance()->checkBit(&cpu_registers[REG_CPSR], Z_POS))
				return false;
			else
				return true;
			break;
		case 2:	//CS/HS
			if(util::getInstance()->checkBit(&cpu_registers[REG_CPSR], C_POS))
				return true;
			else
				return false;
			break;
		case 3:	//CC/LO
			if(util::getInstance()->checkBit(&cpu_registers[REG_CPSR], C_POS))
				return false;
			else
				return true;
			break;
		case 4:	//MI
			if(util::getInstance()->checkBit(&cpu_registers[REG_CPSR], N_POS))
				return true;
			else
				return false;
			break;
		case 5:	//PL
			if(util::getInstance()->checkBit(&cpu_registers[REG_CPSR], N_POS))
				return false;
			else
				return true;
			break;
		case 6:	//VS
			if(util::getInstance()->checkBit(&cpu_registers[REG_CPSR], V_POS))
				return true;
			else
				return false;
			break;
		case 7:	//VC
			if(util::getInstance()->checkBit(&cpu_registers[REG_CPSR], V_POS))
				return false;
			else
				return true;
			break;
		case 8:	//HI
			if(	util::getInstance()->checkBit(&cpu_registers[REG_CPSR], C_POS) &&
				!util::getInstance()->checkBit(&cpu_registers[REG_CPSR], Z_POS))
				return true;
			else
				return false;
			break;
		case 9:	//LS
			if(	!util::getInstance()->checkBit(&cpu_registers[REG_CPSR], C_POS) ||
				util::getInstance()->checkBit(&cpu_registers[REG_CPSR], Z_POS))
				return true;
			else
				return false;
			break;
		case 10:	//GE
			if (util::getInstance()->checkBit(&cpu_registers[REG_CPSR], N_POS) ==
				util::getInstance()->checkBit(&cpu_registers[REG_CPSR], V_POS))
				return true;
			else
				return false;
			break;
		case 11:	//LT
			if (util::getInstance()->checkBit(&cpu_registers[REG_CPSR], N_POS) !=
				util::getInstance()->checkBit(&cpu_registers[REG_CPSR], V_POS))
				return true;
			else
				return false;
			break;
		case 12:	//GT
			if (!util::getInstance()->checkBit(&cpu_registers[REG_CPSR], Z_POS) &&
					(	util::getInstance()->checkBit(&cpu_registers[REG_CPSR], N_POS) ==
						util::getInstance()->checkBit(&cpu_registers[REG_CPSR], V_POS)
					)
				)
				return true;
			else
				return false;
			break;
		case 13:	//LE
			if (util::getInstance()->checkBit(&cpu_registers[REG_CPSR], Z_POS) ||
					(	util::getInstance()->checkBit(&cpu_registers[REG_CPSR], N_POS) !=
						util::getInstance()->checkBit(&cpu_registers[REG_CPSR], V_POS)
					)
				)
				return true;
			else
				return false;
			break;
		case 14:	//AL
			return true;
			break;
		case 15:	//Reserved (should not happen..)
		default:
			return false;
			break;
	}
}

void processor::nextCycle() {
	status = PS_RUNNING;
	if(cpu_registers[REG_CPSR] & T_MASK)	//Thumb state
		*getPC() += 2;
	else 									//ARM state
		*getPC() += 4;
	if(*getPC() % 4 == 0)	//in ARM state or after second halfword in Thumb state, fetch new word
		bus->fetch(getPC());
};

void processor::prefetch() {
	bus->fetch(getPC());
	nextCycle();
}

void processor::debugARM(string mnemonic){
	//cout << "Executing... "<< mnemonic << " (ARM)\n";
	setOP(mnemonic, true);
}

void processor::debugThumb(string mnemonic){
	//cout << "Executing... "<< mnemonic << " (Thumb)\n";
	setOP(mnemonic, false);
}

/* *************************** *
 * 							   *
 * Exceptions & traps handling *
 *                             *
 * *************************** */

void processor::undefinedTrap(){
	execTrap(EXC_UNDEF);
}

void processor::softwareInterruptTrap(){
	execTrap(EXC_SWI);
}

void processor::dataAbortTrap(){
	execTrap(EXC_DATAABT);
}

void processor::resetTrap(){
	execTrap(EXC_RESET);
}

void processor::prefetchAbortTrap(){
	execTrap(EXC_PREFABT);
}

void processor::interruptTrap(){
	execTrap(EXC_IRQ);
}

void processor::fastInterruptTrap(){
	execTrap(EXC_FIQ);
}

void processor::execTrap(ExceptionMode exception){
    Word spsr;
	switch(exception){	//set the right return registers
		case EXC_SWI:
			if(cpu_registers[REG_CPSR] & T_MASK)	//thumb state
				cpu_registers[REG_LR_SVC] = *getPC() - 6;
			else 									//ARM state
				cpu_registers[REG_LR_SVC] = *getPC() - 4;
			spsr = REG_SPSR_SVC;
			break;
		case EXC_UNDEF:
			if(cpu_registers[REG_CPSR] & T_MASK)	//thumb state
				cpu_registers[REG_LR_UND] = *getPC() - 6;
			else 									//ARM state
				cpu_registers[REG_LR_UND] = *getPC() - 4;
			spsr = REG_SPSR_UND;
			break;
		case EXC_DATAABT:
			cpu_registers[REG_LR_ABT] = *getPC();
			spsr = REG_SPSR_ABT;
			break;
		case EXC_RESET:
			cpu_registers[REG_LR_SVC] = get_unpredictable();
			cpu_registers[REG_SPSR_SVC] = get_unpredictable();
			break;
		case EXC_PREFABT:
			cpu_registers[REG_LR_ABT] = *getPC() + 4;
			spsr = REG_SPSR_ABT;
			break;
		case EXC_IRQ:
			if(cpu_registers[REG_CPSR] & T_MASK)	//thumb state
				cpu_registers[REG_LR_IRQ] = *getPC() - 2;
			else 									//ARM state
				cpu_registers[REG_LR_IRQ] = *getPC();
			spsr = REG_SPSR_IRQ;
			break;
		case EXC_FIQ:
			if(cpu_registers[REG_CPSR] & T_MASK)	//thumb state
				cpu_registers[REG_LR_FIQ] = *getPC() - 2;
			else 									//ARM state
				cpu_registers[REG_LR_FIQ] = *getPC();
			spsr = REG_SPSR_FIQ;
			break;
	}
	
	Word *cpsr = getVisibleRegister(REG_CPSR);
	if(exception != EXC_RESET)
		cpu_registers[spsr] = *cpsr;	//spsr contains old cpsr
	*cpsr &= 0xFFFFFFE0;
	
	switch(exception){	//enter privileged mode and edit status register as needed
		case EXC_SWI:
			*cpsr |= MODE_SUPERVISOR;
			util::getInstance()->resetBitReg(cpsr, 5);	//execute in arm state
			util::getInstance()->setBitReg(cpsr, 7);	//disable normal interrupts
			break;
		case EXC_UNDEF:
			*cpsr |= MODE_UNDEFINED;
			break;
		case EXC_DATAABT:
            *cpsr |= MODE_ABORT;
			break;
		case EXC_RESET:
			*cpsr |= MODE_SUPERVISOR | I_MASK | F_MASK;
			*cpsr &= INVERT_W(T_MASK);
			break;
		case EXC_PREFABT:
			*cpsr |= MODE_ABORT;
			break;
		case EXC_IRQ:
			*cpsr |= MODE_INTERRUPT;
			break;
		case EXC_FIQ:
			*cpsr |= MODE_FAST_INTERRUPT;
			break;
	}
	
	*getPC() = exception;
	
	prefetch();
}

void processor::unpredictable(){
	//just for now fill a random number of registers (excluding r13, r14, r15 and PSR) with random words
	Word regmask = rand() % 0x1FFF;
	for(int i = 0; i < 13; i++)
		if(regmask & (1 << i))
			*getVisibleRegister(i) = get_unpredictable();
}

Word processor::get_unpredictable(){
	Word ret;
    for(unsigned i = 0; i < sizeof(Word) * 8; i++)
		ret |= (rand() % 1 ? 1 : 0) << i;
	return ret;
}

bool processor::get_unpredictableB(){
	return rand() % 1;
}

/* ******************** *
 * 						*
 * Instruction decoding *
 * 						*
 * ******************** */

void processor::execute(){
	if(pipeline[PIPELINE_EXECUTE] == OP_HALT){
		status = PS_HALTED;
		return;
	}
	Byte codeHi;
	Byte codeLow;
	
	if(cpu_registers[REG_CPSR] & T_MASK){	// processor in Thumb state
		if(*getPC() & 2){	//read second halfword
			codeHi = (Byte) (pipeline[PIPELINE_EXECUTE] >> 28) & 0xF;
			codeLow = (Byte) (pipeline[PIPELINE_EXECUTE] >> 24) & 0xF;
		} else {
			codeHi = (Byte) (pipeline[PIPELINE_EXECUTE] >> 12) & 0xF;
			codeLow = (Byte) (pipeline[PIPELINE_EXECUTE] >> 8) & 0xF;
		}
		execThumb->execute(codeHi, codeLow);
	}
	else{									// processor in ARM state
		codeHi = (Byte) (pipeline[PIPELINE_EXECUTE] >> 20) & 0xFF;
		codeLow = (Byte) (pipeline[PIPELINE_EXECUTE] >> 4) & 0xF;
		execARM->execute(codeHi, codeLow);
	}
}

/* *************************** *
 * 							   *
 * Istructions implementations *
 * 							   *
 * *************************** */
 
void processor::coprocessorOperation(){
	coprocessor *cp = cpint->getCoprocessor((pipeline[PIPELINE_EXECUTE] >> 8) & 0xF);
	if(cp == NULL){	//no coprocessor can take this command: undefined trap!
		undefinedTrap();
		return;
	}
	Byte opcode = (pipeline[PIPELINE_EXECUTE] >> 20) & 0xF;
	Byte rn = (pipeline[PIPELINE_EXECUTE] >> 16) & 0xF;
	Byte rm = pipeline[PIPELINE_EXECUTE] & 0xF;
	Byte rd = (pipeline[PIPELINE_EXECUTE] >> 12) & 0xF;
	Byte info = (pipeline[PIPELINE_EXECUTE] >> 5) & 7;
	cp->executeOperation(opcode, rm, rn, rd, info);
}


// controllare gli addendi del pc sia in memAcc che no!!!

void processor::coprocessorTransfer(bool memAcc, bool toCoproc){
	Byte cpNum = (pipeline[PIPELINE_EXECUTE] >> 8) & 0xF;
	coprocessor *cp = cpint->getCoprocessor(cpNum);
	if(cp == NULL){	//no coprocessor can take this command: undefined trap!
		undefinedTrap();
		return;
	}
	if(memAcc){
		Word *base = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 16) & 0xF);
		Word cpRegNum = (pipeline[PIPELINE_EXECUTE] >> 12) & 0xF;
		Word *cpReg;
		Byte offset = pipeline[PIPELINE_EXECUTE] & 0xFF;
		bool P, U, W, N;
		P = util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 24);	// pre/post indexing
		U = util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 23);	// up/down
		W = util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 21);	// Write-back
		N = util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 22);	// single/multiple transfer
        Word address = (base == getPC() ? (*base - 4) : *base); 	//if PC is specified as base the value should be the index of current instruction + 8 (== PC - 4)
        if(P)
            address += ((U ? 1 : -1) * (offset << 2));
		if(N){	//multiple transfer
			while((cpReg = cp->getRegister(cpRegNum)) != NULL){	// load all registers starting from cpRegNum
				if(toCoproc){	//load
					*cpReg = bus->getRam()->readW(&address, BIGEND_sig);
				}
				else{			//store
					bus->getRam()->writeW(&address, *cpReg, BIGEND_sig);
				}
				cpRegNum++;
				address += 4;
			}
		}
		else{	//single transfer
			cpReg = cp->getRegister(cpRegNum);
			if(cpReg != NULL){
				if(toCoproc){	//load
					*cpReg = bus->getRam()->read(&address, BIGEND_sig);
				}
				else{			//store
					bus->getRam()->writeW(&address, *cpReg, BIGEND_sig);
				}
			}
		}
        if(!P)
            address += ((U ? 1 : -1) * (offset << 2));
		if(W)
			*base = address;
	}
	else{
		Byte opcode = (pipeline[PIPELINE_EXECUTE] >> 21) & 7;
		Byte info = (pipeline[PIPELINE_EXECUTE] >> 5) & 7;
		Byte rm = pipeline[PIPELINE_EXECUTE] & 0xF;
		Byte rn = (pipeline[PIPELINE_EXECUTE] >> 16) & 0xF;
		Word *srcDest = cp->getRegister(rn);
		Word *rd = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 12) & 0xF);
		if(toCoproc){	//MCR
			*srcDest = *rd + (rd == getPC() ? 8 : 0);	//if pc is source register, stores PC+12
			cp->registerTransfer(opcode, rm, rn, info);
		}
		else{			//MRC
		
			//va comunque eseguito il codice del coprocessore??
			//cp->registerTransfer(opcode, rm, rn, info);
		
			if(rd == getPC()){
				util::getInstance()->copyBitFromReg(&cpu_registers[REG_CPSR], N_POS, srcDest);
				util::getInstance()->copyBitFromReg(&cpu_registers[REG_CPSR], Z_POS, srcDest);
				util::getInstance()->copyBitFromReg(&cpu_registers[REG_CPSR], C_POS, srcDest);
				util::getInstance()->copyBitFromReg(&cpu_registers[REG_CPSR], V_POS, srcDest);
			}
			else{
				*rd = *srcDest;
			}
		}
	}
}

void processor::multiply(bool accumulate, bool lngWord){ //without booth algorythm
	bool S = util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 20);	// set condition flags
	Word *rm, *rs;
	rm = getVisibleRegister(pipeline[PIPELINE_EXECUTE] & 0xF);
	rs = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 8) & 0xF);
		
	if(lngWord){	//multiply (and accumulate) doublewords
		bool U = util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 22);	// unsigned operation
		Word *destHi, *destLo;
		destLo = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 12) & 0xF);
		destHi = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 16) & 0xF);
		
		if(	getPC() == rm || getPC() == rs || getPC() == destHi || getPC() == destLo ||	// r15 can't be used
			rm == destHi || rm == destLo || destHi == destLo){	//destination registers and rm must be all different registers
			unpredictable();
			return;
		}
		
		if(U){
			DoubleWord result = (DoubleWord)*rm * (DoubleWord)*rs;
			if(accumulate){
				DoubleWord accumulator = (DoubleWord)*destLo + ((DoubleWord)*destHi << (sizeof(Word) * 8));
				result += accumulator;
			
			}
			*destLo = result & 0xFFFFFFFF;
			*destHi = (result >> (sizeof(Word)*8)) & 0xFFFFFFFF;
		}
		else{
			SDoubleWord result = (SDoubleWord)((SWord) *rm) * (SDoubleWord)((SWord) *rs);
			if(accumulate){
				SDoubleWord accumulator = (SDoubleWord)((SWord)*destLo) + ((SDoubleWord)((SWord)*destHi) << (sizeof(Word) * 8));
				result += accumulator;
			}
			*destLo = result & 0xFFFFFFFF;
			*destHi = (result >> (sizeof(Word)*8)) & 0xFFFFFFFF;
		}
		
		if(S){
			util::getInstance()->copyBitFromReg(&cpu_registers[REG_CPSR], N_POS, destHi, 31);
			util::getInstance()->copyBitReg(&cpu_registers[REG_CPSR], Z_POS, ((*destHi == 0 && *destLo == 0) ? 1 : 0));
			util::getInstance()->copyBitReg(&cpu_registers[REG_CPSR], C_POS, (get_unpredictableB() ? 1 : 0));	// C and V bit is set to a meaningless value
			util::getInstance()->copyBitReg(&cpu_registers[REG_CPSR], V_POS, (get_unpredictableB() ? 1 : 0));
		}
	}
	else{			//multiply (and accumulate) words
		Word *add, *dest;
		add = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 12) & 0xF);
		dest = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 16) & 0xF);
		
		if(rm == dest || getPC() == rm || getPC() == rs || getPC() == dest || getPC() == add){	// r15 can't be used, destination and first operand can't be the same register
			unpredictable();
			return;
		}
		*dest = (*rm) * (*rs) + (accumulate ? *add : 0);
		if(S){
			util::getInstance()->copyBitFromReg(&cpu_registers[REG_CPSR], N_POS, dest, 31);
			util::getInstance()->copyBitReg(&cpu_registers[REG_CPSR], Z_POS, (*dest == 0 ? 1 : 0));
			util::getInstance()->copyBitReg(&cpu_registers[REG_CPSR], C_POS, (get_unpredictableB() ? 1 : 0));	// C bit is set to a meaningless value
		}
	}
}

void processor::singleDataSwap(){
	Word *src = getVisibleRegister(pipeline[PIPELINE_EXECUTE] & 0xF);
	Word *dest = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 12) & 0xF);
	Word *base = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 16) & 0xF);
	
	if(src == getPC() || dest == getPC() || base == getPC()){
		unpredictable();
		return;
	}
	
	bool B = util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 22);	// swap byte/word
	
	Word tmpRead;
	
	bus->getRam()->lockMem();
	loadStore(true, false, true, B, false, &tmpRead, base, 0);
	loadStore(false, false, true, B, false, src, base, 0);
	*dest = tmpRead;
	bus->getRam()->unlockMem();
}

void processor::blockDataTransfer(bool load){
	Word *base = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 16) & 0xF);
	if(base == getPC()){	//using r15 as base register gives unpredictable results
		unpredictable();
		return;
	}
	HalfWord list = pipeline[PIPELINE_EXECUTE] & 0xFFFF;
	bool P, U, S, W;
	P = util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 24);	// pre/post indexing
	U = util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 23);	// up/down
	S = util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 22);	// PSR & force user
	W = util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 21);	// Write-back
	
	Byte regn;
    for(unsigned i = 0; i < sizeof(HalfWord) * 8; i++)
		if(list & (1<<i))
			regn++;
	
	Word address = *base + ((U ? 1 : -1) * (P ? 4 : 0)) - (U ? 0 : (regn * 4));
	if(load){		//LDM
		if(S){	//user bank transfer / mode change
			if(getMode() == MODE_USER){	// S bit should be set only in privileged mode
				unpredictable();
				return;
			}
            for(unsigned i = 0; i < (sizeof(HalfWord) * 8)-1; i++){
				if(list & (1<<i)){		// if register i is marked load it
					cpu_registers[i] = bus->getRam()->readW(&address);
					address += 4;
				}
			}
			if(list & 0x8000){			// if r15 is required for loading restore also CPSR
				*getPC() = bus->getRam()->readW(&address);
				cpu_registers[REG_CPSR] = *getVisibleRegister(REG_SPSR);
				if(W)
					*base += (U ? 1 : -1) * (regn * 4);
			}
			else{
				if(W){					// base writeback should not be used in this case
					unpredictable();
					return;
				}
			}
		}
		else{	// regular multiple load
            for(unsigned i = 0; i < (sizeof(HalfWord) * 8); i++){
				if(list & (1<<i)){		// if register i is marked load it
					*getVisibleRegister(i) = bus->getRam()->readW(&address);
					address += 4;
				}
			}
		}
	}
	else{			//STM
		if(S){	// user bank transfer
			if(	W ||						// base writeback should not be used in this case
				(getMode() == MODE_USER)){	// S bit should be set only in privileged mode
				unpredictable();
				return;
			}
            for(unsigned i = 0; i < (sizeof(HalfWord) * 8); i++){
				if(list & (1<<i)){		// if register i is marked store it
					bus->getRam()->writeW(&address, cpu_registers[i]);
					address += 4;
				}
			}
		}
		else{	// regular multiple store
			bool firstChecked = false;
            for(unsigned i = 0; i < (sizeof(HalfWord) * 8); i++){
				if(list & (1<<i)){		// if register i is marked store it
					if(!firstChecked){	// if first register to be stored is base address register
						firstChecked = true;
						bus->getRam()->writeW(&address, *getVisibleRegister(i));	// store the initial value before writing back return address
						if(W)
							*base += (U ? 1 : -1) * (regn * 4);
					}
					else
						bus->getRam()->writeW(&address, *getVisibleRegister(i));
					address += 4;
				}
			}
		}
	}
}

void processor::accessPSR(bool load){
	bool spsr = util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 22);
	if(getMode() == MODE_USER && spsr){	//user mode doesn't have SPSR register, unpredictable behavior
		unpredictable();
		return;
	}
	Word *psr = (spsr ? getVisibleRegister(REG_SPSR) : getVisibleRegister(REG_CPSR));
	if(load){	//MRS
		Byte destNum = (pipeline[PIPELINE_EXECUTE] >> 12) & 0xF;
		if(destNum >= 15){	//pc must not be destination register
			unpredictable();
			return;
		}
		else{
			Word *dest = getVisibleRegister(destNum);
			*dest = *psr;
		}
	}
	else{		//MSR
		if(util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 16) && util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 25)){	//if immediate and field bits are set wrong unpredictable behavior
			unpredictable();
			return;
		}
		Word operand;
		if(util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 25)){	//immediate value
			Word val = pipeline[PIPELINE_EXECUTE] & 0xFF;
			Word amount = (pipeline[PIPELINE_EXECUTE] >> 8) & 0xF;
			operand = (val >> amount) | (val << (sizeof(Word)*8 - amount));
		}
		else{	//register value
			if((pipeline[PIPELINE_EXECUTE] & 0xF) < 15)
				operand = *(getVisibleRegister(pipeline[PIPELINE_EXECUTE] & 0xF));
			else{
				unpredictable();
				return;
			}
		}
		if((operand & PSR_UNALLOC_MASK) != 0){	//attempt to set reserved bits
			unpredictable();
			return;
		}
		Word mask = 0xFF000000 | (util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 16) ? 0xFF : 0);
		if(spsr)
			mask &= (PSR_USER_MASK | PSR_PRIV_MASK | PSR_STATE_MASK);
		else{
			if((operand & PSR_STATE_MASK) != 0){	//attempt to set non ARM execution state
				unpredictable();
				return;
			}
			if(getMode() == MODE_USER)
				mask &= PSR_USER_MASK;
			else
				mask &= (PSR_USER_MASK | PSR_PRIV_MASK);
		}
		*psr = (*psr & ! mask) | (operand & mask);
	}
}

void processor::branch(bool link, bool exchange){
	Word *pc = getVisibleRegister(REG_PC);
	if(exchange){
		Word *dest = getVisibleRegister(pipeline[PIPELINE_EXECUTE] & 0xF);
		util::getInstance()->copyBitReg(getVisibleRegister(REG_CPSR), T_POS, (*dest & 1));
		*pc = *dest & 0xFFFFFFFE;
	} else {
		if(link){
			Word *lr = getVisibleRegister(REG_LR);
			*lr = *pc-4;
			*lr &= 0xFFFFFFFC;
		}
		Word offset = (pipeline[PIPELINE_EXECUTE] & 0xFFFFFF) << 2;
		if(offset >> 24 != 0)
			for(int i = 25; i < 32; i++)	//magic numbers, this operation is strictly based on 32bit words..
				offset |= 1<<i;
		*pc += (SWord) offset;
	}
	bus->branchHappened = true;
}

void processor::halfwordDataTransfer(bool sign, bool load_halfwd){
	bool P, U, I, W;
	P = util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 24);
	U = util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 23);
	I = util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 22);
	W = util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 21);
	
	Word *src = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 16) & 0xF);
	Word *dest = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 12) & 0xF);
	Word offset, address = *src;
	if(src == getVisibleRegister(REG_PC))	//if source is r15 the address must be instruction addr+12
		address += 12;
	
	if(I)	//immediate offset
		offset = (pipeline[PIPELINE_EXECUTE] & 0xF) + ((pipeline[PIPELINE_EXECUTE] >> 8) & 0xF);
	else
		offset = *(getVisibleRegister(pipeline[PIPELINE_EXECUTE] & 0xF));
	
	if(P){	//preindexing
		if(U)
			address += offset;
		else
			address -= offset;
	}
	
	if(!(sign && !load_halfwd) && ((address & 1) > 0)) {	//if address is not halfword aligned return unpredictable value
		*dest = get_unpredictable();
	} else {	//address is ok
		if(sign){ //it's a signed load
			Word ret = 0;
			if(load_halfwd){ //load halfword and sign extend
				HalfWord readwd = bus->getRam()->readH(&address, BIGEND_sig);
				ret = readwd;
				if(readwd >> ((sizeof(HalfWord)*8)-1) != 0)
                    for(unsigned i = sizeof(HalfWord)*8; i < sizeof(Word)*8; i ++)
						ret |= 1<<i;
			} else {	//load byte and sign extend
				Byte readwd = bus->getRam()->read(&address, BIGEND_sig);
				ret = readwd;
				if(readwd >> ((sizeof(Byte)*8)-1) != 0)
                    for(unsigned i = sizeof(Byte)*8; i < sizeof(Word)*8; i ++)
						ret |= 1<<i;
			}
			*dest = ret;
		} else {	//it's a halfword transfer
			if(load_halfwd) {	//load val
				*dest = bus->getRam()->readH(&address, BIGEND_sig);
			} else {	//store val
				bus->getRam()->writeH(&address, (HalfWord) (*dest & 0xFFFF), BIGEND_sig);
			}
		}
	}
	
	if(P) {	//preindexing
		if(W)	//Writeback
			*src = address;
	} else { //postindexing always writes back
		if(U)
			*src = address + offset;
		else
			*src = address - offset;
	}
}

void processor::singleMemoryAccess(bool L){
	Word *base = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 16) & 0xF);
	Word *srcDst = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 12) & 0xF);
	Word offset;
	if((pipeline[PIPELINE_EXECUTE] & (1 << 25)) > 0){	//I flag
		barrelShifter(false, ((pipeline[PIPELINE_EXECUTE] >> 4) & 0xFF), (pipeline[PIPELINE_EXECUTE] & 0xF));
		offset = shifter_operand;
	} else
		offset = pipeline[PIPELINE_EXECUTE] & 0xFFF;
	bool P, U, B, W;
	P = util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 24);
	U = util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 23);
	B = util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 22);
	W = util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 21);
	
	loadStore(L, P, U, B, W, srcDst, base, offset);
}

/*
 * accesso alla memoria di rifare, tutte le funzioni devono ritornare un bool che, 
 * se falso, segnala un abort, e in quel caso si lancia la trap!!!!
 */ 

void processor::loadStore(bool L, bool P, bool U, bool B, bool W, Word* srcDst, Word* base, Word offset){
	Word address = *base;
	if(P){	//pre-indexing
		address += ((U ? 1 : -1) * offset);
			
		if(B){
			if(L)
				*srcDst = bus->getRam()->read(&address, BIGEND_sig);
			else
				bus->getRam()->write(&address, ((Byte) *srcDst & 0xFF), BIGEND_sig);
		} else
			if(L)
				*srcDst = bus->getRam()->readW(&address, BIGEND_sig);
			else
				bus->getRam()->writeW(&address, *srcDst, BIGEND_sig);
		if(W)
			*base = address;
	}
	else{
		if(W)	//user-mode forced transfer (only available in privileged mode): use user mode registers
			srcDst = getRegister((pipeline[PIPELINE_EXECUTE] >> 12) & 0xF);
		if(B){
			if(L)
				*srcDst = bus->getRam()->read(base, BIGEND_sig);
			else
				bus->getRam()->write(base, ((Byte) *srcDst & 0xFF), BIGEND_sig);
		} else {
			if(L)
				*srcDst = bus->getRam()->readW(&address, BIGEND_sig);
			else
				bus->getRam()->writeW(&address, *srcDst, BIGEND_sig);
		}
		*base = address + ((U ? 1 : -1) * offset);
	}
}

void processor::dataProcessing(Byte opcode){
	if((pipeline[PIPELINE_EXECUTE] & (1 << 25)) > 0)
		barrelShifter(true, (pipeline[PIPELINE_EXECUTE] & 0xFF), ((pipeline[PIPELINE_EXECUTE] >> 8) & 0xF));
	else
		barrelShifter(false, ((pipeline[PIPELINE_EXECUTE] >> 4) & 0xFF), (pipeline[PIPELINE_EXECUTE] & 0xF));
	Word op1 = *(getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 16) & 0xF));
	Word *dest = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 12) & 0xF);
	Word op2 = shifter_operand;
	bool S = pipeline[PIPELINE_EXECUTE] & (1<<20);
	switch(opcode){
		case 8:		//TST
			S = true;	//this instruction always updates CPSR
			dest = &alu_tmp;
		case 0:		//AND
			*dest = op1 & op2;
			bitwiseReturn(dest, S);
			break;
		case 9:		//TEQ
			S = true;	//this instruction always updates CPSR
			dest = &alu_tmp;
		case 1:		//EOR
			*dest = op1 ^ op2;
			bitwiseReturn(dest, S);
			break;
		case 10:	//CMP
			S = true;	//this instruction always updates CPSR
			dest = &alu_tmp;
		case 2:		//SUB
			dataPsum(op1, op2, false, false, dest, S);
			break;
		case 3:		//RSB
			dataPsum(op2, op1, false, false, dest, S);
			break;
		case 11:	//CMN
			S = true;	//this instruction always updates CPSR
			dest = &alu_tmp;
		case 4:		//ADD
			dataPsum(op1, op2, false, true, dest, S);
			break;
		case 5:		//ADC
			dataPsum(op1, op2, true, true, dest, S);
			break;
		case 6:		//SBC
			dataPsum(op1, op2, true, false, dest, S);
			break;
		case 7:		//RSC
			dataPsum(op2, op1, true, false, dest, S);
			break;
		case 12:	//ORR
			*dest = op1 | op2;
			bitwiseReturn(dest, S);
			break;
		case 15:	//MVN
			op2 = INVERT_W(op2);
		case 13:	//MOV
			*dest = op2;
			bitwiseReturn(dest, S);
			break;
		case 14:	//BIC
			*dest = op1 & (INVERT_W(op2));
			bitwiseReturn(dest, S);
			break;
	}
}

void processor::dataPsum(Word op1, Word op2, bool carry, bool sum, Word *dest, bool S){
	int64_t sres;
	bool overflow = false, borrow = false;
	uint64_t c = (carry && util::getInstance()->checkBit(getVisibleRegister(REG_CPSR), C_POS) ? 1 : 0);
	if(sum){
		uint64_t ures;
		sres = (SDoubleWord)((SWord)op1)+(SDoubleWord)((SWord)op2)+c;
		ures = (DoubleWord)op1+(DoubleWord)op2+c;
		if(ures > 0xFFFFFFFF)
			borrow = true;
	} else {
		sres = (SDoubleWord)((SWord)op1)-(SDoubleWord)((SWord)op2)-c;
		if(op1<(op2+c))
			borrow = true;
	}
	if(sres > (SDoubleWord)0x7FFFFFFF || sres < (SDoubleWord)0xFFFFFFFF80000000)
		overflow = true;
	*dest = (Word) sres & 0xFFFFFFFF;
	if(S){	// S == 1
		if(dest == getPC()){
			Word *savedPSR = getVisibleRegister(REG_SPSR);
			if(savedPSR != NULL)
				cpu_registers[REG_CPSR] = *savedPSR;
			else
				unpredictable();
		} else {
			util::getInstance()->copyBitFromReg(&cpu_registers[REG_CPSR], N_POS, dest, 31);
			util::getInstance()->copyBitReg(&cpu_registers[REG_CPSR], Z_POS, (*dest == 0 ? 1 : 0));
			util::getInstance()->copyBitReg(&cpu_registers[REG_CPSR], C_POS, (borrow ? 1 : 0));
			util::getInstance()->copyBitReg(&cpu_registers[REG_CPSR], V_POS, (overflow ? 1 : 0));
		}
	}
}

void processor::bitwiseReturn(Word *dest, bool S){
	if(S){
		if(dest == getPC()){
			Word *savedPSR = getVisibleRegister(REG_SPSR);
			if(savedPSR != NULL)
				cpu_registers[REG_CPSR] = *savedPSR;
			else
				unpredictable();
		} else {
			util::getInstance()->copyBitFromReg(&cpu_registers[REG_CPSR], N_POS, dest, 31);
			util::getInstance()->copyBitReg(&cpu_registers[REG_CPSR], Z_POS, (*dest == 0 ? 1 : 0));
			util::getInstance()->copyBitReg(&cpu_registers[REG_CPSR], C_POS, (shifter_carry_out ? 1 : 0));
		}
	}
}

#endif //UARM_PROCESSOR_CC
