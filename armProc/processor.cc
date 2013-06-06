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

processor::processor() : pu() {
	status = PS_IDLE;
	BIGEND_sig = false;
	cpint = new coprocessor_interface();
	cpu_registers[REG_CPSR] = MODE_USER;
	/*for(int i = 0; i < CPU_REGISTERS_NUM; i++)
		cpu_registers[i] = 0;*/
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
	bus->fetch(getPC());
	cpu_registers[REG_PC] += 4;
};

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
	if(condCheck()){
		Byte bits = (pipeline[PIPELINE_EXECUTE] >> 21) & 0x7F;
		switch(bits>>5){
			case 0:
				if(util::getInstance()->checkBits(&pipeline[PIPELINE_EXECUTE], BX_MASK) && util::getInstance()->checkBits(INVERT_W(pipeline[PIPELINE_EXECUTE]), NBX_MASK))
					BX();
				else if(util::getInstance()->checkBits(&pipeline[PIPELINE_EXECUTE], SWP_MASK) && util::getInstance()->checkBits(INVERT_W(pipeline[PIPELINE_EXECUTE]), NSWP_MASK))
					SWP();
				else if((util::getInstance()->checkBits(&pipeline[PIPELINE_EXECUTE], MUL_MASK) && util::getInstance()->checkBits(INVERT_W(pipeline[PIPELINE_EXECUTE]), NMUL_MASK)) ||
						(util::getInstance()->checkBits(&pipeline[PIPELINE_EXECUTE], LMUL_MASK) && util::getInstance()->checkBits(INVERT_W(pipeline[PIPELINE_EXECUTE]), NLMUL_MASK)))
					multiply();
				else if((util::getInstance()->checkBits(&pipeline[PIPELINE_EXECUTE], HWT_MASK) && util::getInstance()->checkBits(INVERT_W(pipeline[PIPELINE_EXECUTE]), NHWT_MASK)) ||
						(util::getInstance()->checkBits(&pipeline[PIPELINE_EXECUTE], HWTI_MASK) && util::getInstance()->checkBits(INVERT_W(pipeline[PIPELINE_EXECUTE]), NHWTI_MASK)))
					halfwordDataTransfer();
				else
					dataProcessing();
				break;
			case 1:
				if(util::getInstance()->checkBits(&pipeline[PIPELINE_EXECUTE], ((1<<4)|(3<<25))))
					undefined();
				else
					singleDataTransfer();
				break;
			case 2:
				if(util::getInstance()->checkBit(&pipeline[PIPELINE_EXECUTE], 25))
					branch();
				else
					blockDataTransfer();
				break;
			case 3:
				if(util::getInstance()->checkBits(&pipeline[PIPELINE_EXECUTE], 0xF<<24))
					SWI();
				else
					coprocessorInstr();
				break;
		}
	} else
		NOP();
}

void processor::dataProcessing(){
	Byte opcode = (Byte) (pipeline[PIPELINE_EXECUTE] >> 21) & 0xF;
	if((pipeline[PIPELINE_EXECUTE] & (1 << 25)) > 0)
		barrelShifter(true, (pipeline[PIPELINE_EXECUTE] & 0xFF), ((pipeline[PIPELINE_EXECUTE] >> 8) & 0xF));
	else
		barrelShifter(false, ((pipeline[PIPELINE_EXECUTE] >> 4) & 0xFF), (pipeline[PIPELINE_EXECUTE] & 0xF));
	Word op1 = *(getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 16) & 0xF));
	Word *dest = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 12) & 0xF);
	Word op2 = shifter_operand;
	switch(opcode){
		case 0:
			AND(op1, op2, dest);
			break;
		case 1:
			EOR(op1, op2, dest);
			break;
		case 2:
			SUB(op1, op2, dest);
			break;
		case 3:
			RSB(op1, op2, dest);
			break;
		case 4:
			ADD(op1, op2, dest);
			break;
		case 5:
			ADC(op1, op2, dest);
			break;
		case 6:
			SBC(op1, op2, dest);
			break;
		case 7:
			RSC(op1, op2, dest);
			break;
		case 8:
			TST(op1, op2, dest);
			break;
		case 9:
			TEQ(op1, op2, dest);
			break;
		case 10:
			CMP(op1, op2, dest);
			break;
		case 11:
			CMN(op1, op2, dest);
			break;
		case 12:
			ORR(op1, op2, dest);
			break;
		case 13:
			MOV(op1, op2, dest);
			break;
		case 14:
			BIC(op1, op2, dest);
			break;
		case 15:
			MVN(op1, op2, dest);
			break;
	}
}

void processor::multiply(){
	
}

void processor::singleDataSwap(){
	
}

void processor::halfwordDataTransfer(){
	
}

void processor::branch(){
	
}

void processor::coprocessorInstr(){
	
}

void processor::singleDataTransfer(){
	Word offset;
	if((pipeline[PIPELINE_EXECUTE] & (1 << 25)) > 0){	//I flag
		barrelShifter(false, ((pipeline[PIPELINE_EXECUTE] >> 4) & 0xFF), (pipeline[PIPELINE_EXECUTE] & 0xF));
		offset = shifter_operand;
	} else
		offset = pipeline[PIPELINE_EXECUTE] & 0xFFF;
	bool P, U, B, W, L;
	P = util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 24);
	U = util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 23);
	B = util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 22);
	W = util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 21);
	L = util::getInstance()->checkBit(pipeline[PIPELINE_EXECUTE], 20);
	Word *src = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 16) & 0xF);
	Word *dest = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 12) & 0xF);
	if(L)
		LDR(P, U, B, W, src, offset, dest);
	else
		STR(P, U, B, W, src, offset, dest);
}

void processor::blockDataTransfer(){
	
}

void processor::undefined(){
	
}

void processor::unpredictable(){
	
}

/* *************************** *
 * 							   *
 * Istructions implementations *
 * 							   *
 * *************************** */

void processor::ADC(Word op1, Word op2, Word *dest){
	dataPsum(op1, op2, true, true, dest);
}

void processor::ADD(Word op1, Word op2, Word *dest){	
	dataPsum(op1, op2, false, true, dest);
}

void processor::AND(Word op1, Word op2, Word *dest){
	*dest = op1 & op2;
	bitwiseReturn(dest);
}

void processor::BIC(Word op1, Word op2, Word *dest){
	*dest = op1 & (INVERT_W(op2));
	bitwiseReturn(dest);
}

void processor::BX(){
	
}

void processor::CMN(Word op1, Word op2, Word *dest){
	pipeline[PIPELINE_EXECUTE] |= 1<<20;	//this instruction always updates CPSR
	ADD(op1, op2, &alu_tmp);
}

void processor::CMP(Word op1, Word op2, Word *dest){
	pipeline[PIPELINE_EXECUTE] |= 1<<20;	//this instruction always updates CPSR
	SUB(op1, op2, &alu_tmp);
}

void processor::EOR(Word op1, Word op2, Word *dest){
	*dest = op1 ^ op2;
	bitwiseReturn(dest);
}

void processor::LDR(bool P, bool U, bool B, bool W, Word *src, Word offset, Word *dest){
	singleMemoryAccess(true, P, U, B, W, src, offset, dest);
}

void processor::MOV(Word op1, Word op2, Word *dest){
	*dest = op2;
	bitwiseReturn(dest);
}

void processor::MVN(Word op1, Word op2, Word *dest){
	MOV(op1, INVERT_W(op2), dest);
}

void processor::ORR(Word op1, Word op2, Word *dest){
	*dest = op1 | op2;
	bitwiseReturn(dest);
}

void processor::RSB(Word op1, Word op2, Word *dest){
	SUB(op2, op1, dest);
}

void processor::RSC(Word op1, Word op2, Word *dest){
	SBC(op2, op1, dest);
}

void processor::SBC(Word op1, Word op2, Word *dest){
	dataPsum(op1, op2, true, false, dest);
}

void processor::STR(bool P, bool U, bool B, bool W, Word *src, Word offset, Word *data){
	singleMemoryAccess(false, P, U, B, W, src, offset, data);
}

void processor::SUB(Word op1, Word op2, Word *dest){
	dataPsum(op1, op2, false, false, dest);
}

void processor::SWI(){
	
}

void processor::SWP(){
	
}

void processor::TEQ(Word op1, Word op2, Word *dest){
	pipeline[PIPELINE_EXECUTE] |= 1<<20;	//this instruction always updates CPSR
	EOR(op1, op2, &alu_tmp);
}

void processor::TST(Word op1, Word op2, Word *dest){
	pipeline[PIPELINE_EXECUTE] |= 1<<20;	//this instruction always updates CPSR
	AND(op1, op2, &alu_tmp);
}

void processor::singleMemoryAccess(bool L, bool P, bool U, bool B, bool W, Word *src, Word offset, Word *dest){
		Word address = *src;
	if(U)
		address += offset;
	else
		address -= offset;
	if(P){	//pre-indexing
		if(B){
			if(L)
				*dest = bus->getRam()->read(&address, BIGEND_sig);
			else
				bus->getRam()->write(&address, ((Byte) *dest & 0xFF), BIGEND_sig);
		} else
			if(L)
				*dest = bus->getRam()->readW(&address, BIGEND_sig);
			else
				bus->getRam()->writeW(&address, *dest, BIGEND_sig);
		if(W)
			*src = address;
	}
	else{
		if(W);	//user-mode forced transfer (only available in privileged mode)
		if(B){
			if(L)
				*dest = bus->getRam()->read(src, BIGEND_sig);
			else
				bus->getRam()->write(src, ((Byte) *dest & 0xFF), BIGEND_sig);
		} else
			if(L)4
				*dest = bus->getRam()->readW(&address, BIGEND_sig);
			else
				bus->getRam()->writeW(&address, *dest, BIGEND_sig);
		*src = address;
	}
}

void processor::dataPsum(Word op1, Word op2, bool carry, bool sum, Word *dest){
	int64_t sres;
	bool overflow = false, borrow = false;
	if(sum){
		uint64_t ures;
		uint64_t c = (carry && util::getInstance()->checkBit(getVisibleRegister(REG_CPSR), C_POS) ? 1 : 0);
		sres = (int64_t)((int32_t)op1)+(int64_t)((int32_t)op2)+c;
		ures = (uint64_t)op1+(uint64_t)op2+c;
		if(ures > 0xFFFFFFFF)
			borrow = true;
	} else {
		int64_t ures;
		uint64_t c = (carry && !(util::getInstance()->checkBit(getVisibleRegister(REG_CPSR), C_POS)) ? 1 : 0);
		sres = (int64_t)((int32_t)op1)-(int64_t)((int32_t)op2)-c;
		ures = (int64_t)((uint32_t)op1-(uint32_t)op2-c);
		if(ures < 0)
			borrow = true;
	}
	if(sres > (int64_t)0x7FFFFFFF || sres < (int64_t)0xFFFFFFFF80000000)
		overflow = true;
	*dest = (Word) sres & 0xFFFFFFFF;
	if(pipeline[PIPELINE_EXECUTE] & (1<<20)){	// S == 1
		if(((pipeline[PIPELINE_EXECUTE] >> 16) & 0xF) == 15){	//Rd == 15
			Word *savedPSR = getVisibleRegister(REG_SPSR);
			if(savedPSR != NULL)
				cpu_registers[REG_CPSR] = *savedPSR;
			else
				unpredictable();
		} else {
			util::getInstance()->copyBitFromReg(getVisibleRegister(REG_CPSR), N_POS, dest, 31);
			util::getInstance()->copyBitReg(getVisibleRegister(REG_CPSR), Z_POS, (*dest == 0 ? 1 : 0));
			util::getInstance()->copyBitReg(getVisibleRegister(REG_CPSR), C_POS, (borrow ? 1 : 0));
			util::getInstance()->copyBitReg(getVisibleRegister(REG_CPSR), V_POS, (overflow ? 1 : 0));
		}
	}
}

void processor::bitwiseReturn(Word *dest){
	if(pipeline[PIPELINE_EXECUTE] & (1<<20)){	// S == 1
		if(((pipeline[PIPELINE_EXECUTE] >> 16) & 0xF) == 15){	//Rd == 15
			Word *savedPSR = getVisibleRegister(REG_SPSR);
			if(savedPSR != NULL)
				cpu_registers[REG_CPSR] = *savedPSR;
			else
				unpredictable();
		} else {
			util::getInstance()->copyBitFromReg(getVisibleRegister(REG_CPSR), N_POS, dest, 31);
			util::getInstance()->copyBitReg(getVisibleRegister(REG_CPSR), Z_POS, (*dest == 0 ? 1 : 0));
			util::getInstance()->copyBitReg(getVisibleRegister(REG_CPSR), C_POS, (shifter_carry_out ? 1 : 0));
		}
	}
}

#endif //UARM_PROCESSOR_CC
