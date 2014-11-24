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

#include "armProc/processor.h"
#include "services/util.h"
#include "armProc/ARMisa.h"
#include "armProc/Thumbisa.h"
#include "services/error.h"

processor::processor(systemBus *bus) : pu(bus) {
    //cpint = new coprocessor_interface(bus);
    coproc = new cp15(bus);

	execARM = new ARMisa(this);
	execThumb = new Thumbisa(this);

    pipeline = bus->pipeline;

    reset();
}

processor::~processor(){
    if(coproc != NULL){
        delete coproc;
        coproc = NULL;
    }
    if(execARM != NULL){
        delete execARM;
        execARM = NULL;
    }
    if(execThumb != NULL){
        delete execThumb;
        execThumb = NULL;
    }
    delete [] cpu_registers;
    delete [] prefetchFault;
    tlb.~scoped_array();
}

void processor::reset(){
    status = PS_RESET;
    old_pc = 0;
    branch_happened = false;
    BIGEND_sig = ENDIANESS_BIGENDIAN;

    for(int i = 0; i < CPU_REGISTERS_NUM; i++)
        cpu_registers[i] = 0;

    cpu_registers[REG_CPSR] = MODE_SUPERVISOR;

    for(int i = 0; i < PIPELINE_STAGES; i ++)
        prefetchFault[i] = false;

    coproc->reset();

    tlbSize = MC_Holder::getInstance()->getConfig()->getTLBSize();
    if(tlb)
        tlb.~scoped_array();
    tlb.reset(new TLBEntry[tlbSize]);

    Word address = 0;
    bus->writeW(&address, INITIAL_BRANCH, true);
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
        Panic("Invalid Processor Mode\n");
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
                    shifter_operand = val;
                    shifter_carry_out = checkBit(cpu_registers[REG_CPSR], C_POS);
                } else if(amount >= 32) {
                    shifter_operand = ((val >> 31) == 1 ? 0xFFFFFFFF : 0);
                    shifter_carry_out = ((val >> 31) == 1 ? true : false);
                } else {
					Word ret = val >> amount;
					if((val >> 31) == 1)
						ret |= 0xFFFFFFFF << (32 - amount);
					else
                        ret &= 0xFFFFFFFF >> amount;
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
    return condCheck(cond);
}

bool processor::condCheck(Byte cond){
	switch(cond){
		case 0:	//EQ
            if(checkBit(&cpu_registers[REG_CPSR], Z_POS))
				return true;
			else
				return false;
			break;
		case 1:	//NE
            if(checkBit(&cpu_registers[REG_CPSR], Z_POS))
				return false;
			else
				return true;
			break;
		case 2:	//CS/HS
            if(checkBit(&cpu_registers[REG_CPSR], C_POS))
				return true;
			else
				return false;
			break;
		case 3:	//CC/LO
            if(checkBit(&cpu_registers[REG_CPSR], C_POS))
				return false;
			else
				return true;
			break;
		case 4:	//MI
            if(checkBit(&cpu_registers[REG_CPSR], N_POS))
				return true;
			else
				return false;
			break;
		case 5:	//PL
            if(checkBit(&cpu_registers[REG_CPSR], N_POS))
				return false;
			else
				return true;
			break;
		case 6:	//VS
            if(checkBit(&cpu_registers[REG_CPSR], V_POS))
				return true;
			else
				return false;
			break;
		case 7:	//VC
            if(checkBit(&cpu_registers[REG_CPSR], V_POS))
				return false;
			else
				return true;
			break;
		case 8:	//HI
            if(	checkBit(&cpu_registers[REG_CPSR], C_POS) &&
                !checkBit(&cpu_registers[REG_CPSR], Z_POS))
				return true;
			else
				return false;
			break;
		case 9:	//LS
            if(	!checkBit(&cpu_registers[REG_CPSR], C_POS) ||
                checkBit(&cpu_registers[REG_CPSR], Z_POS))
				return true;
			else
				return false;
			break;
		case 10:	//GE
            if (checkBit(&cpu_registers[REG_CPSR], N_POS) ==
                checkBit(&cpu_registers[REG_CPSR], V_POS))
				return true;
			else
				return false;
			break;
		case 11:	//LT
            if (checkBit(&cpu_registers[REG_CPSR], N_POS) !=
                checkBit(&cpu_registers[REG_CPSR], V_POS))
				return true;
			else
				return false;
			break;
		case 12:	//GT
            if (!checkBit(&cpu_registers[REG_CPSR], Z_POS) &&
                    (	checkBit(&cpu_registers[REG_CPSR], N_POS) ==
                        checkBit(&cpu_registers[REG_CPSR], V_POS)
					)
				)
				return true;
			else
				return false;
			break;
		case 13:	//LE
            if (checkBit(&cpu_registers[REG_CPSR], Z_POS) ||
                    (	checkBit(&cpu_registers[REG_CPSR], N_POS) !=
                        checkBit(&cpu_registers[REG_CPSR], V_POS)
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
    if(status == PS_RESET){     //if this is the first life cycle, enter reset state
        resetTrap();
        prefetch();
        return;
    }                           //else run normally
    Word ppc;
    if(cpu_registers[REG_CPSR] & T_MASK){	//Thumb state
        *getPC() += 2;
    }
    else{ 									//ARM state
        *getPC() += 4;
    }
    old_pc = *getPC();
    if(*getPC() % 4 == 0){	//in ARM state or after second halfword in Thumb state, fetch new word
        prefetchFault[PIPELINE_EXECUTE] = prefetchFault[PIPELINE_DECODE];
        prefetchFault[PIPELINE_DECODE] = prefetchFault[PIPELINE_FETCH];
        if(mapVirtual(*getPC(), &ppc, EXEC)){
            prefetchFault[PIPELINE_FETCH] = true;
        } else
            prefetchFault[PIPELINE_FETCH] = !bus->fetch(ppc, !(cpu_registers[REG_CPSR] & T_MASK));
    }
}

void processor::prefetch() {
    Word ppc, vpc = *getPC();
    int i;
    for(i = PIPELINE_EXECUTE; i >= PIPELINE_FETCH; i--, vpc+=4){
        if(mapVirtual(vpc, &ppc, EXEC)){
            prefetchFault[i] = true;
        } else
            prefetchFault[i] = !bus->prefetch(ppc);
    }
    //if prefetch happened in thumb mode pc must be advanced by two halfwords
    *getPC() += (cpu_registers[REG_CPSR] & T_MASK ? 4 : 8);
    old_pc = *getPC();
}

bool processor::branchHappened(){
    if(old_pc != *getPC() || branch_happened)
        return true;
    else
        return false;
}

void processor::debugARM(string mnemonic){
	//cout << "Executing... "<< mnemonic << " (ARM)\n";
	setOP(mnemonic, true);
}

void processor::debugThumb(string mnemonic){
	//cout << "Executing... "<< mnemonic << " (Thumb)\n";
	setOP(mnemonic, false);
}

void processor::AssertIRQ(unsigned int il)
{
    *(coproc->getIPCauseRegister()) |= CAUSE_IP(il);

    // If in standby mode, go back to being a power hog.
    if (status == PS_IDLE){
        status = PS_RUNNING;
    }
}

void processor::DeassertIRQ(unsigned int il)
{
    *(coproc->getIPCauseRegister()) &= ~CAUSE_IP(il);
}

void processor::cycle() {
    wasException = false;
    Word intm = bus->getPendingInt(this);  //STATIC: you must pass cpu id to get the right interrupts if multiprocessor is implemented
    if(CAUSE_IP_MASK & intm){
        if(((intm >> (24 + IL_TIMER)) & 1) && timerEnabled()) // Fast interrupt!
            fastInterruptTrap();
        else if(interruptsEnabled())
            interruptTrap();
    }
    fetch();
    branch_happened = false;
    decode();
    setOP("Unknown", true);
    execute();
}

void processor::fetch(){
    if(branchHappened()){
        prefetch();
    } else {
        nextCycle();
    }
}

// This method maps the virtual addresses to physical ones following the
// complex mapping algorithm and TLB used by MIPS (see external doc).
// It returns TRUE if conversion was not possible (this implies an exception
// have been raised) and FALSE if conversion has taken place: physical value
// for address conversion is returned thru paddr pointer.
// AccType details memory access type (READ/WRITE/EXECUTE)
bool processor::mapVirtual(Word vaddr, Word * paddr, Word accType)
{
    if (coproc->isVMon()) {
        // VM is on

        // SignalProcVAccess() is always done so it is possible
        // to track accesses which produce exceptions
        bus->HandleVMAccess(ENTRYHI_GET_ASID(*coproc->getRegister(CP15_REG2_EntryHi)), vaddr, accType, this);

        // address validity and bounds check
        if (BADADDR(vaddr) || ((getMode() == MODE_USER) && (vaddr < USEG2BASE))) {
            // bad offset or kernel segment access from user mode
            *paddr = MAXWORDVAL;

            // the bad virtual address is put into BADVADDR reg
            *coproc->getRegister(CP15_REG6_FA) = vaddr;

            if (accType == WRITE)
                coproc->setCause(ADESEXCEPTION);
            else
                coproc->setCause(ADELEXCEPTION);

            return true;
        }

        // The access is in user mode to user space, or in kernel mode
        // to KSEG0 or USEG2/3 spaces.

        unsigned int index;
        if (probeTLB(&index, *coproc->getRegister(CP15_REG2_EntryHi), vaddr)) {
            if (tlb[index].IsV()) {
                if (accType != WRITE || tlb[index].IsD()) {
                    // All OK
                    *paddr = PHADDR(vaddr, tlb[index].getLO());
                    return false;
                } else {
                    // write operation on frame with D bit set to 0
                    *paddr = MAXWORDVAL;
                    setTLBRegs(vaddr);
                    coproc->setCause(MODEXCEPTION);
                    return true;
                }
            } else  {
                // invalid access to frame with V bit set to 0
                *paddr = MAXWORDVAL;
                setTLBRegs(vaddr);
                if (accType == WRITE)
                    coproc->setCause(TLBSEXCEPTION);
                else
                    coproc->setCause(TLBLEXCEPTION);

                return true;
            }
        } else {
            // bad or missing VPN match: Refill event required
            *paddr = MAXWORDVAL;
            setTLBRegs(vaddr);
            if (accType == WRITE)
                coproc->setCause(UTLBSEXCEPTION);
            else
                coproc->setCause(UTLBLEXCEPTION);

            return true;
        }
    } else {
        // VM is off

        // SignalProcVAccess() is always done so it is possible
        // to track accesses which produce exceptions
        bus->HandleVMAccess(MAXASID, vaddr, accType, this);

        // address validity and bounds check
        if (BADADDR(vaddr) || ((getMode() == MODE_USER) && (vaddr < RAM_BASE))) {
            // bad offset or kernel segment access from user mode
            *paddr = MAXWORDVAL;

            // the bad address is put into BADVADDR reg
            *coproc->getRegister(CP15_REG6_FA) = vaddr;

            if (accType == WRITE)
                coproc->setCause(ADESEXCEPTION);
            else
                coproc->setCause(ADELEXCEPTION);

            return true;
        } else {
            *paddr = vaddr;
            return false;
        }
    }
}

// This method sets the CP0 special registers on exceptions forced by TLB
// handling (see mapVirtual() for invocation/specific cases).
void processor::setTLBRegs(Word vaddr)
{
    // Note that ENTRYLO is left undefined!
    *coproc->getRegister(CP15_REG6_FA) = vaddr;
    *coproc->getRegister(CP15_REG2_EntryHi) = VPN(vaddr) | ASID(*coproc->getRegister(CP15_REG2_EntryHi));
}

// This method scans the TLB looking for a entry that matches ASID/VPN pair;
// scan algorithm follows MIPS specifications, and returns the _highest_
// entry that matches
bool processor::probeTLB(unsigned int* index, Word asid, Word vpn)
{
    bool found = false;

    for (unsigned int i = 0; i < tlbSize; i++) {
        if (tlb[i].VPNMatch(vpn) && (tlb[i].IsG() || tlb[i].ASIDMatch(asid))) {
            found = true;
            *index = i;
        }
    }

    return found;
}

void processor::getTLB(unsigned int index, Word* hi, Word* lo) const
{
    *hi = tlb[index].getHI();
    *lo = tlb[index].getLO();
}

Word processor::getTLBHi(unsigned int index) const
{
    return tlb[index].getHI();
}

Word processor::getTLBLo(unsigned int index) const
{
    return tlb[index].getLO();
}

void processor::setTLB(unsigned int index, Word hi, Word lo)
{
    if (index < tlbSize) {
        tlb[index].setHI(hi);
        tlb[index].setLO(lo);
        //SignalTLBChanged(index);
    } else {
        Panic("Unknown TLB entry in Processor::setTLB()");
    }
}

void processor::setTLBHi(unsigned int index, Word value)
{
    assert(index < tlbSize);
    tlb[index].setHI(value);
    //SignalTLBChanged(index);
}

void processor::setTLBLo(unsigned int index, Word value)
{
    assert(index < tlbSize);
    tlb[index].setLO(value);
    //SignalTLBChanged(index);
}

/* *************************** *
 * 							   *
 * Exceptions & traps handling *
 *                             *
 * *************************** */

void processor::undefinedTrap(){
    coproc->setCause(ABT_UNDEFEXCEPTION);
	execTrap(EXC_UNDEF);
}

void processor::softwareInterruptTrap(){
    coproc->setCause(ABT_SYSEXCEPTION);
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
    coproc->setCause(ABT_INTEXCEPTION);
	execTrap(EXC_IRQ);
}

void processor::fastInterruptTrap(){
    coproc->setCause(ABT_INTEXCEPTION);
	execTrap(EXC_FIQ);
}

void processor::execTrap(ExceptionMode exception){
    Word spsr;
	switch(exception){	//set the right return registers
		case EXC_SWI:
			if(cpu_registers[REG_CPSR] & T_MASK)	//thumb state
                cpu_registers[REG_LR_SVC] = *getPC() - 2;
			else 									//ARM state
				cpu_registers[REG_LR_SVC] = *getPC() - 4;
			spsr = REG_SPSR_SVC;
			break;
		case EXC_UNDEF:
			if(cpu_registers[REG_CPSR] & T_MASK)	//thumb state
                cpu_registers[REG_LR_UND] = *getPC() - 2;
			else 									//ARM state
				cpu_registers[REG_LR_UND] = *getPC() - 4;
			spsr = REG_SPSR_UND;
			break;
        case EXC_DATAABT:   // !! check return address
            if(cpu_registers[REG_CPSR] & T_MASK)	//thumb state
                cpu_registers[REG_LR_UND] = *getPC() + 4;
            else 									//ARM state
                cpu_registers[REG_LR_UND] = *getPC();
            spsr = REG_SPSR_ABT;
			break;
		case EXC_RESET:
            cpu_registers[REG_LR_SVC] = BIOSBASEADDR;
            cpu_registers[REG_SPSR_SVC] = bus->get_unpredictable();
			break;
        case EXC_PREFABT:   // !! check return address
            if(cpu_registers[REG_CPSR] & T_MASK)	//thumb state
                cpu_registers[REG_LR_UND] = *getPC() + 4;
            else 									//ARM state
                cpu_registers[REG_LR_UND] = *getPC() + 4;
            spsr = REG_SPSR_ABT;
			break;
        case EXC_IRQ:   // !! check return address
			if(cpu_registers[REG_CPSR] & T_MASK)	//thumb state
                cpu_registers[REG_LR_IRQ] = *getPC() + (branchHappened() ? 6 : 2);  //EDIT: check this!!
			else 									//ARM state                    
                cpu_registers[REG_LR_IRQ] = *getPC() + (branchHappened() ? 4 : 0);
			spsr = REG_SPSR_IRQ;
			break;
        case EXC_FIQ:   // !! check return address
			if(cpu_registers[REG_CPSR] & T_MASK)	//thumb state
                cpu_registers[REG_LR_FIQ] = *getPC() + (branchHappened() ? 6 : 2);  //EDIT: check this!!
			else 									//ARM state
                cpu_registers[REG_LR_FIQ] = *getPC() + (branchHappened() ? 4 : 0);
			spsr = REG_SPSR_FIQ;
			break;
	}
	
	Word *cpsr = getVisibleRegister(REG_CPSR);
	if(exception != EXC_RESET)
		cpu_registers[spsr] = *cpsr;	//spsr contains old cpsr
	*cpsr &= 0xFFFFFFE0;
	
    resetBitReg(cpsr, T_POS);	//execute in arm state
    setBitReg(cpsr, I_POS);	//disable normal interrupts
    setBitReg(cpsr, F_POS); //disable fast interrupts

	switch(exception){	//enter privileged mode and edit status register as needed
		case EXC_SWI:
			*cpsr |= MODE_SUPERVISOR;
			break;
		case EXC_UNDEF:
			*cpsr |= MODE_UNDEFINED;
			break;
		case EXC_DATAABT:
            *cpsr |= MODE_ABORT;
			break;
		case EXC_RESET:
            *cpsr |= MODE_SUPERVISOR;
            status = PS_RUNNING;
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
    old_pc = 0xFFFFFFFF;

    wasException = true;
}

void processor::unpredictable(){
	//just for now fill a random number of registers (excluding r13, r14, r15 and PSR) with random words
	Word regmask = rand() % 0x1FFF;
	for(int i = 0; i < 13; i++)
		if(regmask & (1 << i))
            *getVisibleRegister(i) = bus->get_unpredictable();
}

bool processor::checkAbort(AbortType memSig){
    if(memSig == ABT_NOABT)
        return true;

    coproc->setCause(memSig);

    return false;
}

/* ******************** *
 * 						*
 * Instruction decoding *
 * 						*
 * ******************** */

void processor::execute(){

    if(prefetchFault[PIPELINE_EXECUTE]){
        prefetchAbortTrap();
        return;
    }

	Byte codeHi;
	Byte codeLow;
	
	if(cpu_registers[REG_CPSR] & T_MASK){	// processor in Thumb state
        HalfWord instr;
		if(*getPC() & 2){	//read second halfword
			codeHi = (Byte) (pipeline[PIPELINE_EXECUTE] >> 28) & 0xF;
			codeLow = (Byte) (pipeline[PIPELINE_EXECUTE] >> 24) & 0xF;
            instr = (HalfWord) (pipeline[PIPELINE_EXECUTE] >> 16) & 0xFFFF;
		} else {
			codeHi = (Byte) (pipeline[PIPELINE_EXECUTE] >> 12) & 0xF;
			codeLow = (Byte) (pipeline[PIPELINE_EXECUTE] >> 8) & 0xF;
            instr = (HalfWord) pipeline[PIPELINE_EXECUTE] & 0xFFFF;
		}
        execThumb->execute(codeHi, codeLow, instr);
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
    coprocessor *cp = coproc; //cpint->getCoprocessor((pipeline[PIPELINE_EXECUTE] >> 8) & 0xF);
	if(cp == NULL){	//no coprocessor can take this command: undefined trap!
		undefinedTrap();
		return;
	}
	Byte opcode = (pipeline[PIPELINE_EXECUTE] >> 20) & 0xF;
	Byte rn = (pipeline[PIPELINE_EXECUTE] >> 16) & 0xF;
	Byte rm = pipeline[PIPELINE_EXECUTE] & 0xF;
	Byte rd = (pipeline[PIPELINE_EXECUTE] >> 12) & 0xF;
	Byte info = (pipeline[PIPELINE_EXECUTE] >> 5) & 7;
    switch(pipeline[PIPELINE_EXECUTE]){
        case WAITCPINSTR:
            suspend(); break;
        case HALTCPINSTR:
            halt(); break;
        /* TODO: TLB functions here */
        default:
            if(cp->executeOperation(opcode, rm, rn, rd, info))
                dataAbortTrap();
            break;
    }
}

void processor::coprocessorTransfer(bool memAcc, bool toCoproc){
    //Byte cpNum = (pipeline[PIPELINE_EXECUTE] >> 8) & 0xF;
    coprocessor *cp = coproc;//cpint->getCoprocessor(cpNum);
	if(cp == NULL){	//no coprocessor can take this command: undefined trap!
		undefinedTrap();
		return;
	}
	if(memAcc){
        Word paddr, *base = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 16) & 0xF);
		Word cpRegNum = (pipeline[PIPELINE_EXECUTE] >> 12) & 0xF;
        Word *cpReg;
		Byte offset = pipeline[PIPELINE_EXECUTE] & 0xFF;
		bool P, U, W, N;
        P = checkBit(pipeline[PIPELINE_EXECUTE], 24);	// pre/post indexing
        U = checkBit(pipeline[PIPELINE_EXECUTE], 23);	// up/down
        W = checkBit(pipeline[PIPELINE_EXECUTE], 21);	// Write-back
        N = checkBit(pipeline[PIPELINE_EXECUTE], 22);	// single/multiple transfer
        Word address = *base; //(base == getPC() ? (*base - 4) : *base); 	//if PC is specified as base the value should be the index of current instruction + 8 (== PC - 4)
        if(P)
            address += ((U ? 1 : -1) * (offset << 2));
		if(N){	//multiple transfer
			while((cpReg = cp->getRegister(cpRegNum)) != NULL){	// load all registers starting from cpRegNum
				if(toCoproc){	//load
                    if(mapVirtual(address, &paddr, READ) ||
                            !checkAbort(bus->readW(&paddr, cpReg))){
                        dataAbortTrap();
                        return;
                    }
				}
				else{			//store
                    if(mapVirtual(address, &paddr, WRITE) ||
                            !checkAbort(bus->writeW(&paddr, *cpReg, true))){
                        dataAbortTrap();
                        return;
                    }
				}
				cpRegNum++;
				address += 4;
			}
		}
		else{	//single transfer
			cpReg = cp->getRegister(cpRegNum);
			if(cpReg != NULL){
				if(toCoproc){	//load
                    if(mapVirtual(address, &paddr, READ) ||
                            !checkAbort(bus->readW(&paddr, cpReg))){
                        dataAbortTrap();
                        return;
                    }
				}
				else{			//store
                    if(mapVirtual(address, &paddr, WRITE) ||
                            !checkAbort(bus->writeW(&paddr, *cpReg, true))){
                        dataAbortTrap();
                        return;
                    }
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
        Word *rd = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 12) & 0xF);
		if(toCoproc){	//MCR
            Word send = *rd;
            if(rd == getPC())   //if pc is source register, stores PC+12 (or PC+4???)
                send += 12;
            if(cp->registerTransfer(&send, opcode, rm, rn, info, toCoproc))
                dataAbortTrap();
		}
		else{			//MRC
            Word rec;
            if(cp->registerTransfer(&rec, opcode, rm, rn, info, toCoproc))
                dataAbortTrap();
		
			if(rd == getPC()){
                copyBitFromReg(&cpu_registers[REG_CPSR], N_POS, &rec);
                copyBitFromReg(&cpu_registers[REG_CPSR], Z_POS, &rec);
                copyBitFromReg(&cpu_registers[REG_CPSR], C_POS, &rec);
                copyBitFromReg(&cpu_registers[REG_CPSR], V_POS, &rec);
			}
			else{
                *rd = rec;
			}
		}
	}
}


void processor::multiply(bool accumulate, bool lngWord){
    bool S = checkBit(pipeline[PIPELINE_EXECUTE], 20);	// set condition flags
    Word *rm, *rs, *rd, *rn;
    rm = getVisibleRegister(pipeline[PIPELINE_EXECUTE] & 0xF);
    rs = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 8) & 0xF);
    rn = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 12) & 0xF);
    rd = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 16) & 0xF);
    multiply(rd, rm, rs, rn, accumulate, lngWord, S);
}

void processor::multiply(Word *rd, Word *rm, Word *rs, Word *rn , bool accumulate, bool lngWord, bool S){ //without booth algorythm
	if(lngWord){	//multiply (and accumulate) doublewords
        bool U = checkBit(pipeline[PIPELINE_EXECUTE], 22);	// unsigned operation
		Word *destHi, *destLo;
        destLo = rn;
        destHi = rd;
		
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
            copyBitFromReg(&cpu_registers[REG_CPSR], N_POS, destHi, 31);
            copyBitReg(&cpu_registers[REG_CPSR], Z_POS, ((*destHi == 0 && *destLo == 0) ? 1 : 0));
            copyBitReg(&cpu_registers[REG_CPSR], C_POS, (bus->get_unpredictableB() ? 1 : 0));	// C and V bit is set to a meaningless value
            copyBitReg(&cpu_registers[REG_CPSR], V_POS, (bus->get_unpredictableB() ? 1 : 0));
		}
	}
	else{			//multiply (and accumulate) words
        if(rm == rd || getPC() == rm || getPC() == rs || getPC() == rd || getPC() == rn){	// r15 can't be used, destination and first operand can't be the same register
			unpredictable();
			return;
		}
        *rd = (*rm) * (*rs) + (accumulate ? *rn : 0);
        if(S){
            copyBitFromReg(&cpu_registers[REG_CPSR], N_POS, rd, 31);
            copyBitReg(&cpu_registers[REG_CPSR], Z_POS, (*rd == 0 ? 1 : 0));
            copyBitReg(&cpu_registers[REG_CPSR], C_POS, (bus->get_unpredictableB() ? 1 : 0));	// C bit is set to a meaningless value
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
	
    bool B = checkBit(pipeline[PIPELINE_EXECUTE], 22);	// swap byte/word
	
	Word tmpRead;
	
	bus->getRam()->lockMem();
	loadStore(true, false, true, B, false, &tmpRead, base, 0);
	loadStore(false, false, true, B, false, src, base, 0);
    *dest = tmpRead;
	bus->getRam()->unlockMem();
}

void processor::blockDataTransfer(bool load){
    Word *base = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 16) & 0xF);
    HalfWord list = pipeline[PIPELINE_EXECUTE] & 0xFFFF;
    bool P, U, S, W;
    P = checkBit(pipeline[PIPELINE_EXECUTE], 24);	// pre/post indexing
    U = checkBit(pipeline[PIPELINE_EXECUTE], 23);	// up/down
    S = checkBit(pipeline[PIPELINE_EXECUTE], 22);	// PSR & force user
    W = checkBit(pipeline[PIPELINE_EXECUTE], 21);	// Write-back

    blockDataTransfer(base, list, load, P, U, S, W);
}

void processor::blockDataTransfer(Word *rn, HalfWord list, bool load, bool P, bool U, bool S, bool W){

    if(rn == getPC()){	//using r15 as base register gives unpredictable results
		unpredictable();
		return;
	}

    Byte regn = 0;
    for(unsigned i = 0; i < sizeof(HalfWord) * 8; i++)
		if(list & (1<<i))
            regn++;

    Word writeBackAddr = *rn + ((U ? 1 : -1) * 4 * regn);

    //first address points always to lower most stored register
    Word paddr, address = *rn + ((U ? 1 : -1) * (P ? 4 : 0)) - (U ? 0 : ((regn - 1) * 4));
	if(load){		//LDM
		if(S){	//user bank transfer / mode change
			if(getMode() == MODE_USER){	// S bit should be set only in privileged mode
				unpredictable();
				return;
			}
            for(unsigned i = 0; i < 15; i++){
				if(list & (1<<i)){		// if register i is marked load it
                    if(mapVirtual(address, &paddr, READ) ||
                            !checkAbort(bus->readW(&paddr, &cpu_registers[i]))){
                        dataAbortTrap();
                        return;
                    }
					address += 4;
				}
            }
            if(list & 0x8000){			// if r15 is required for loading restore also CPSR
                if(mapVirtual(address, &paddr, READ) ||
                        !checkAbort(bus->readW(&paddr, getPC()))){
                    dataAbortTrap();
                    return;
                }
                cpu_registers[REG_CPSR] = *getVisibleRegister(REG_SPSR);
            } else {
                if(W){					// base writeback should not be used in this case
                    unpredictable();
                    return;
                }
            }
		}
		else{	// regular multiple load
            for(unsigned i = 0; i < (sizeof(HalfWord) * 8); i++){
                if(list & (1<<i)){		// if register i is marked load it
                    if(mapVirtual(address, &paddr, READ) ||
                            !checkAbort(bus->readW(&paddr, getVisibleRegister(i)))){
                        dataAbortTrap();
                        return;
                    }
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
                    if(mapVirtual(address, &paddr, WRITE) ||
                            !checkAbort(bus->writeW(&paddr, cpu_registers[i], true))){
                        dataAbortTrap();
                        return;
                    }
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
                        if(mapVirtual(address, &paddr, WRITE) ||
                                !checkAbort(bus->writeW(&paddr, *getVisibleRegister(i), true))){   // store the initial value before writing back return address
                            dataAbortTrap();
                            return;
                        }
                    } else
                        if(mapVirtual(address, &paddr, WRITE) ||
                                !checkAbort(bus->writeW(&paddr, *getVisibleRegister(i), true))){
                            dataAbortTrap();
                            return;
                        }

					address += 4;
				}
			}
		}
	}
    if(W)
        *rn = writeBackAddr;
}

void processor::accessPSR(bool load){
    bool spsr = checkBit(pipeline[PIPELINE_EXECUTE], 22);
    if((getMode() == MODE_USER || getMode() == MODE_SYSTEM) && spsr){	//user/system mode doesn't have SPSR register, unpredictable behavior
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
        if(checkBit(pipeline[PIPELINE_EXECUTE], 16) && checkBit(pipeline[PIPELINE_EXECUTE], 25)){	//if immediate and field bits are set wrong unpredictable behavior
			unpredictable();
			return;
		}

		Word operand;
        if(checkBit(pipeline[PIPELINE_EXECUTE], 25)){	//immediate value
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

        if(checkBit(coproc->getRegister(CP15_REG1_SCB), CP15_REG1_L4POS)){   //T bit modifications are ignored
            copyBitFromReg(&operand, T_POS, psr, T_POS);
        }

        Word mask = 0xFF000000 | (checkBit(pipeline[PIPELINE_EXECUTE], 16) ? 0xFF : 0);
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
    Word *dest = getVisibleRegister(pipeline[PIPELINE_EXECUTE] & 0xF);
    Word offset = (pipeline[PIPELINE_EXECUTE] & 0xFFFFFF) << 2;
    branch(dest, offset, link, exchange);
    branch_happened = true;
}

void processor::branch(Word *rd, Word offset, bool link, bool exchange){
	Word *pc = getVisibleRegister(REG_PC);
	if(exchange){
        copyBitReg(getVisibleRegister(REG_CPSR), T_POS, (*rd & 1));
        *pc = *rd & 0xFFFFFFFE;
	} else {
		if(link){
			Word *lr = getVisibleRegister(REG_LR);
			*lr = *pc-4;
			*lr &= 0xFFFFFFFC;
		}
		if(offset >> 24 != 0)
            for(unsigned i = 25; i < (sizeof(Word) * 8); i++)	//magic numbers, this operation is strictly based on 32bit words..
                offset |= (1<<i);
		*pc += (SWord) offset;
    }
}


void processor::halfwordDataTransfer(bool sign, bool load_halfwd){
    bool P, U, I, W;
    P = checkBit(pipeline[PIPELINE_EXECUTE], 24);
    U = checkBit(pipeline[PIPELINE_EXECUTE], 23);
    I = checkBit(pipeline[PIPELINE_EXECUTE], 22);
    W = checkBit(pipeline[PIPELINE_EXECUTE], 21);

    Word *src = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 16) & 0xF);
    Word *dest = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 12) & 0xF);
    Word offset = (pipeline[PIPELINE_EXECUTE] & 0xF) + ((pipeline[PIPELINE_EXECUTE] >> 8) & 0xF);
    Word *rm = getVisibleRegister(pipeline[PIPELINE_EXECUTE] & 0xF);

    halfwordDataTransfer(dest, src, rm, offset, P, U, I, W, sign, load_halfwd);
}
void processor::halfwordDataTransfer(Word *rd, Word *rn, Word *rm, Word offs, bool P, bool U, bool I, bool W, bool sign, bool load_halfwd){
    Word paddr, address = *rn;
    Word offset;
    if(rn == getVisibleRegister(REG_PC))	//if source is r15 the address must be instruction addr+12
		address += 12;
	
	if(I)	//immediate offset
        offset = offs;
	else
        offset = *rm;
	
	if(P){	//preindexing
		if(U)
			address += offset;
		else
			address -= offset;
	}
	
	if(!(sign && !load_halfwd) && ((address & 1) > 0)) {	//if address is not halfword aligned return unpredictable value
        *rd = bus->get_unpredictable();
	} else {	//address is ok
		if(sign){ //it's a signed load
			Word ret = 0;
			if(load_halfwd){ //load halfword and sign extend
                HalfWord readwd;
                if(mapVirtual(address, &paddr, READ) ||
                        !checkAbort(bus->readH(&paddr, &readwd))){
                    dataAbortTrap();
                    return;
                }
				ret = readwd;
				if(readwd >> ((sizeof(HalfWord)*8)-1) != 0)
                    for(unsigned i = sizeof(HalfWord)*8; i < sizeof(Word)*8; i ++)
						ret |= 1<<i;
			} else {	//load byte and sign extend
                Byte readwd;
                if(mapVirtual(address, &paddr, READ) ||
                        !checkAbort(bus->readB(&paddr, &readwd))){
                    dataAbortTrap();
                    return;
                }
				ret = readwd;
				if(readwd >> ((sizeof(Byte)*8)-1) != 0)
                    for(unsigned i = sizeof(Byte)*8; i < sizeof(Word)*8; i ++)
						ret |= 1<<i;
			}
            *rd = ret;
		} else {	//it's a halfword transfer
			if(load_halfwd) {	//load val
                HalfWord readwd;
                if(!mapVirtual(address, &paddr, READ) ||
                        checkAbort(bus->readH(&paddr, &readwd))){
                    dataAbortTrap();
                    return;
                }
                *rd = readwd;
			} else {	//store val
                if(mapVirtual(address, &paddr, WRITE) ||
                        !checkAbort(bus->writeH(&paddr, (HalfWord) (*rd & 0xFFFF), true))){
                    dataAbortTrap();
                    return;
                }
			}
		}
    }
	
	if(P) {	//preindexing
		if(W)	//Writeback
            *rn = address;
	} else { //postindexing always writes back
		if(U)
            *rn = address + offset;
		else
            *rn = address - offset;
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
    P = checkBit(pipeline[PIPELINE_EXECUTE], 24);
    U = checkBit(pipeline[PIPELINE_EXECUTE], 23);
    B = checkBit(pipeline[PIPELINE_EXECUTE], 22);
    W = checkBit(pipeline[PIPELINE_EXECUTE], 21);
	
	loadStore(L, P, U, B, W, srcDst, base, offset);
}


void processor::loadStore(bool L, bool P, bool U, bool B, bool W, Word* srcDst, Word* base, Word offset){
    Word paddr, address = *base;
	if(P){	//pre-indexing
		address += ((U ? 1 : -1) * offset);
			
		if(B){
            if(L){
                Byte read;
                if(mapVirtual(address, &paddr, READ) ||
                        !checkAbort(bus->readB(&paddr, &read))){
                    dataAbortTrap();
                    return;
                }
                *srcDst = read;
            }
            else{
                if(mapVirtual(address, &paddr, WRITE) ||
                        !checkAbort(bus->writeB(&paddr, ((Byte) *srcDst & 0xFF), true))){
                    dataAbortTrap();
                    return;
                }
            }
		} else
            if(L){
                if(mapVirtual(address, &paddr, READ) ||
                        !checkAbort(bus->readW(&paddr, srcDst))){
                    dataAbortTrap();
                    return;
                }
            }
            else{
                if(mapVirtual(address, &paddr, WRITE) ||
                        !checkAbort(bus->writeW(&paddr, *srcDst, true))){
                    dataAbortTrap();
                    return;
                }
            }
        if(W && base != getPC())
			*base = address;
	}
	else{
		if(W)	//user-mode forced transfer (only available in privileged mode): use user mode registers
			srcDst = getRegister((pipeline[PIPELINE_EXECUTE] >> 12) & 0xF);
		if(B){
            if(L){
                Byte read;
                if(mapVirtual(*base, &paddr, READ) ||
                        !checkAbort(bus->readB(&paddr, &read))){
                    dataAbortTrap();
                    return;
                }
                *srcDst = read;
            }
            else{
                if(mapVirtual(*base, &paddr, WRITE) ||
                        !checkAbort(bus->writeB(&paddr, ((Byte) *srcDst & 0xFF), true))){
                    dataAbortTrap();
                    return;
                }
            }
		} else {
            if(L){
                if(mapVirtual(address, &paddr, READ) ||
                        !checkAbort(bus->readW(&paddr, srcDst))){
                    dataAbortTrap();
                    return;
                }
            }
            else{
                if(mapVirtual(address, &paddr, WRITE) ||
                        !checkAbort(bus->writeW(&paddr, *srcDst, true))){
                    dataAbortTrap();
                    return;
                }
            }
		}
        if(base != getPC())
            *base = address + ((U ? 1 : -1) * offset);
    }
}

void processor::dataProcessing(Byte opcode){
    bool I = checkBit(pipeline[PIPELINE_EXECUTE], 25);
    Word op1 = *(getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 16) & 0xF));
    Word *dest = getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 12) & 0xF);
    if(I)
		barrelShifter(true, (pipeline[PIPELINE_EXECUTE] & 0xFF), ((pipeline[PIPELINE_EXECUTE] >> 8) & 0xF));
    else{
        barrelShifter(false, ((pipeline[PIPELINE_EXECUTE] >> 4) & 0xFF), (pipeline[PIPELINE_EXECUTE] & 0xF));
        if(checkBit(pipeline[PIPELINE_EXECUTE], 4)){       //if shift amount is specified in a register
            if(getVisibleRegister((pipeline[PIPELINE_EXECUTE] >> 16) & 0xF))    //and PC is used it, should have value PC+4
                op1 += 4;
            else if(dest == getPC())
                *dest += 4;
        }
    }
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
    *dest = 0;
    bool overflow = false;
    bool borrow = carry && checkBit(getVisibleRegister(REG_CPSR), C_POS);
    Word sop2 = op2;
    if(!sum){
        sop2 ^= 0xFFFFFFFF;
        borrow = true;
    }
    for(uint i = 0; i < sizeof(Word)*8; i++){
        switch((checkBit(op1, i) ? 1 : 0) + (checkBit(sop2, i) ? 1 : 0) + (borrow ? 1 : 0)){
            case 1:
                *dest |= (1 << i);
            case 0:
                borrow = false;
                break;
            case 3:
                *dest |= (1 << i);
            case 2:
                borrow = true;
                break;
        }
    }
    if((checkBit(op1, 31) == checkBit(sop2, 31))
      && (checkBit(op1, (31)) != checkBit(op1, (31))))
        overflow = true;
	if(S){	// S == 1
		if(dest == getPC()){
			Word *savedPSR = getVisibleRegister(REG_SPSR);
            if(savedPSR != NULL)
				cpu_registers[REG_CPSR] = *savedPSR;
			else
                unpredictable();
		} else {
            copyBitFromReg(&cpu_registers[REG_CPSR], N_POS, dest, 31);
            copyBitReg(&cpu_registers[REG_CPSR], Z_POS, (*dest == 0 ? 1 : 0));
            copyBitReg(&cpu_registers[REG_CPSR], C_POS, (borrow ? 1 : 0));
            copyBitReg(&cpu_registers[REG_CPSR], V_POS, (overflow ? 1 : 0));
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
            copyBitFromReg(&cpu_registers[REG_CPSR], N_POS, dest, 31);
            copyBitReg(&cpu_registers[REG_CPSR], Z_POS, (*dest == 0 ? 1 : 0));
            copyBitReg(&cpu_registers[REG_CPSR], C_POS, (shifter_carry_out ? 1 : 0));
		}
	}
}

#endif //UARM_PROCESSOR_CC
