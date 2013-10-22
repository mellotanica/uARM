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

void Thumbisa::execute(Byte hi, Byte low, HalfWord instr){
	
    (*this.*main_table[hi][low])(instr);
}

Word Thumbisa::shift(Word val, Word amount, Byte type){
    switch(type){
        case SHIFT_LSL:
            if(amount == 0){
                p->shifter_carry_out = ((*(p->getRegister(REG_CPSR)) & C_MASK) == 1 ? true : false);
                return val;
            } else {
                p->shifter_carry_out = ((val & (1<<(32-amount))) == 1 ? true : false);
                return (val << amount);
            }
            break;
        case SHIFT_LSR:
            if(amount == 0){	//special case, represents LSR #32
                p->shifter_carry_out = ((val >> 31) == 1 ? true : false);
                return 0;
            } else {
                p->shifter_carry_out = ((val & 1<<(amount-1)) == 1 ? true : false);
                return (val >> amount);
            }
            break;
        case SHIFT_ASR:
            if(amount == 0){	//special case, represents ASR #32
                p->shifter_carry_out = ((val >> 31) == 1 ? true : false);
                return ((val >> 31) == 1 ? 0xFFFFFFFF : 0);
            } else {
                Word ret = val >> amount;
                if((val >> 31) == 1)
                    ret |= 0xFFFFFFFF << (32 - amount);
                else
                    ret &= 0xFFFFFFFF >> (31 - amount);
                p->shifter_carry_out = ((val & 1<<(amount-1)) == 1 ? true : false);
                return ret;
            }
            break;
        case SHIFT_ROR:
            if(amount == 0){    //special case: rotate right extended
                p->shifter_carry_out = ((val & 1) == 1 ? true : false);
                return (val >> 1 | ((*(p->getRegister(REG_CPSR)) & C_MASK) << 31));
            } else {
                Word ret = ((val >> amount) | (val << (sizeof(Word)*8 - amount)));
                p->shifter_carry_out = ((ret >> 31) == 1 ? true : false);
                return ret;
            }
            break;
    default: return 0; break;
    }
}

 /* *************** *
  * 				*
  *    Thumb ISA	*
  * 				*
  * *************** */

void Thumbisa::ADD(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Word *rd, *rs, offset;

    if(instr & (1<<13)){    //format 2
        rd = p->getVisibleRegister((Byte)instr & 7);
        rs = p->getVisibleRegister((Byte)(instr >> 3) & 7);
        offset = (instr >> 6) & 7;
        if(!(instr & (1<<10))){    //register offset
            offset = *(p->getVisibleRegister((Byte)offset));
        }
    } else {                //format 3
        rs = rd = p->getVisibleRegister((Byte)(instr >> 8) & 7);
        offset = instr & 0xFF;
    }

    p->dataPsum(*rs, offset, false, true, rd, true);
}

void Thumbisa::ADDH(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Word *rd, *rs;
    Byte Hs = (instr >> 6) & 3;
    switch(Hs){
        case 1: //h2
            rd = p->getVisibleRegister((Byte)instr & 7);
            rs = p->getVisibleRegister((Byte)((instr >> 3) & 7) + 8);
            break;
        case 2: //h1
            rd = p->getVisibleRegister((Byte)(instr & 7) + 8);
            rs = p->getVisibleRegister((Byte)(instr >> 3) & 7);
        case 3: //h1&h2
            rd = p->getVisibleRegister((Byte)(instr & 7) + 8);
            rs = p->getVisibleRegister((Byte)((instr >> 3) & 7) + 8);
        default:
            p->unpredictable();
            return;
            break;
    }

    p->dataPsum(*rd, *rs, false, true, rd, false);
}

void Thumbisa::ADDPC(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Word *rd = p->getVisibleRegister((Byte)(instr >> 8) & 7);
    Word offset = (instr & 0xff) << 2;

    p->dataPsum(*(p->getPC()), offset, false, true, rd, false);
}

void Thumbisa::ADDSP(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    if(instr & (1<<12)){    //format 13 (add offset to sp)
        bool sum = (instr >> 7) & 1;
        Word offset = (instr & 0x7f) << 2;
        Word *sp = p->getVisibleRegister(REG_SP);

        p->dataPsum(*sp, offset, false, sum, sp, false);
    } else {                //format 12 (load sp address with offset)
        Word *rd = p->getVisibleRegister((Byte)(instr >> 8) & 7);
        Word offset = (instr & 0xff) << 2;
        Word *sp = p->getVisibleRegister(REG_SP);

        p->dataPsum(*sp, offset, false, true, rd, false);
    }
}

void Thumbisa::ASR(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Word *rd = p->getVisibleRegister((Byte)(instr & 7));
    Word rs = *(p->getVisibleRegister((Byte)((instr >> 3) & 7)));
    Byte offset = (instr >> 6) & 0x1F;

    *rd = shift(rs, offset, SHIFT_ASR);
    p->bitwiseReturn(rd, true);  //MOVS
}

void Thumbisa::B(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Word offset = (instr & 0x7FF) << 1;
    if(offset >> 10 != 0)
        for(unsigned i = 11; i < (sizeof(Word) * 8); i++)
            offset |= (1<<i);

    p->branch(NULL, offset, false, false);
}

void Thumbisa::Bcond(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Byte cond = (Byte)(instr >> 8) & 0xF;
    if(cond == 14){
        p->unpredictable();
        return;
    }
    if(cond == 15){
        SWI(instr);
        return;
    }

    if(p->condCheck(cond)){
        Word offset = (instr & 0xFF) << 1;
        if(offset >> 8 != 0)
            for(unsigned i = 9; i < (sizeof(Word) * 8); i++)
                offset |= (1<<i);

        p->branch(NULL, offset, false, false);
    }
}

void Thumbisa::BL(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Word offset = instr & 0x7FE;
    if((instr >> 11) & 1){  //stage 2
        Word *lr = p->getVisibleRegister(REG_LR);
        Word tmp = (*lr + (offset << 1)) & 0xFFFFFFFC;
        *lr = (*(p->getPC()) - 2) | 1;

        p->branch(&tmp, 0, false, true);
    } else {                //stage 1
        offset = offset << 12;
        Word *lr = p->getVisibleRegister(REG_LR);
        *lr = (*(p->getPC()) + offset) | 1;
    }
}

void Thumbisa::BX(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Word *rs;
    Byte Hs = (instr >> 6) & 3;
    switch(Hs){
        case 0: //r2
            rs = p->getVisibleRegister((Byte)(instr >> 3) & 7);
            break;
        case 1: //h2
            rs = p->getVisibleRegister((Byte)((instr >> 3) & 7) + 8);
        default:
            p->unpredictable();
            return;
            break;
    }

    p->branch(rs, 0, false, true);
}

void Thumbisa::CMP(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Word *rd= p->getVisibleRegister((Byte)(instr >> 8) & 7);
    Word offset = instr & 0xFF;

    p->dataPsum(*rd, offset, false, false, &(p->alu_tmp), true);
}

void Thumbisa::CMPH(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Word *rd, *rs;
    Byte Hs = (instr >> 6) & 3;
    switch(Hs){
        case 1: //h2
            rd = p->getVisibleRegister((Byte)instr & 7);
            rs = p->getVisibleRegister((Byte)((instr >> 3) & 7) + 8);
            break;
        case 2: //h1
            rd = p->getVisibleRegister((Byte)(instr & 7) + 8);
            rs = p->getVisibleRegister((Byte)(instr >> 3) & 7);
        case 3: //h1&h2
            rd = p->getVisibleRegister((Byte)(instr & 7) + 8);
            rs = p->getVisibleRegister((Byte)((instr >> 3) & 7) + 8);
        default:
            p->unpredictable();
            return;
            break;
    }

    p->dataPsum(*rd, *rs, false, false, &(p->alu_tmp), true);
}

void Thumbisa::DP(HalfWord instr){	//data processing, triggers an ALU operation
	p->debugThumb(__FUNCTION__);
	
    Byte hi = (instr >> 8) & 3;
    Byte low = (instr >> 6) & 3;
    Word *rd = p->getVisibleRegister((Byte)instr&7);
    Word *rs = p->getVisibleRegister((Byte)(instr >> 3) & 7);

        (*this.*ALU_table[hi][low])(rd, rs);	//execute ALU operation
}

void Thumbisa::LDMIA(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Word *rb = p->getVisibleRegister((Byte)(instr >> 8) & 7);
    HalfWord list = instr & 0xFF;

    p->blockDataTransfer(rb, list, true, false, true, false, true);
}

void Thumbisa::LDR(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Word *rb = p->getVisibleRegister((Byte)(instr >> 3) & 7);
    Word *rd = p->getVisibleRegister((Byte)instr & 7);
    Word offset;

    if(instr & (1<<13)){    //format 9 (immediate offset)
        offset = ((instr >> 6) & 0x1F) << 2;
    } else {                //format 7 (register offset)
        offset = *(p->getVisibleRegister((Byte)(instr >> 6) & 7));
    }

    p->loadStore(true, true, true, false, false, rd, rb, offset);
}

void Thumbisa::LDRB(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Word *rb = p->getVisibleRegister((Byte)(instr >> 3) & 7);
    Word *rd = p->getVisibleRegister((Byte)instr & 7);
    Word offset;

    if(instr & (1<<13)){    //format 9 (immediate offset)
        offset = (instr >> 6) & 0x1F;
    } else {                //format 7 (register offset)
        offset = *(p->getVisibleRegister((Byte)(instr >> 6) & 7));
    }

    p->loadStore(true, true, true, true, false, rd, rb, offset);
}

void Thumbisa::LDRH(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Word *rb = p->getVisibleRegister((Byte)(instr >> 3) & 7);
    Word *rd = p->getVisibleRegister((Byte)instr & 7);

    if(instr & (1<<13)){    //format 10 (immediate offset)
        Word offset = ((instr >> 6) & 0x1F) << 1;

        p->halfwordDataTransfer(rd, rb, NULL, offset, true, true, true, false, false, true);
    } else {                //format 8 (register offset)
        Word *ro = p->getVisibleRegister((Byte)(instr >> 6) & 7);

        p->halfwordDataTransfer(rd, rb, ro, 0, true, true, false, false, false, true);
    }
}

void Thumbisa::LDRPC(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Word *rd = p->getVisibleRegister((Byte)(instr >> 8) & 7);
    Word baseAddr = *p->getPC() & INVERT_W(3);
    Word offset = (instr & 0xFF) << 2;

    p->loadStore(true, true, true, false, false, rd, &baseAddr, offset);
}

void Thumbisa::LDRSB(HalfWord instr){
    p->debugThumb(__FUNCTION__);

    Word *ro = p->getVisibleRegister((Byte)(instr >> 6) & 7);
    Word *rb = p->getVisibleRegister((Byte)(instr >> 3) & 7);
    Word *rd = p->getVisibleRegister((Byte)instr & 7);

    p->halfwordDataTransfer(rd, rb, ro, 0, true, true, false, false, true, false);
}

void Thumbisa::LDRSH(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Word *ro = p->getVisibleRegister((Byte)(instr >> 6) & 7);
    Word *rb = p->getVisibleRegister((Byte)(instr >> 3) & 7);
    Word *rd = p->getVisibleRegister((Byte)instr & 7);

    p->halfwordDataTransfer(rd, rb, ro, 0, true, true, false, false, true, true);
}

void Thumbisa::LDRSP(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Word *rd = p->getVisibleRegister((Byte)(instr >> 8) & 7);
    Word *rn = p->getVisibleRegister(REG_SP);
    Word offset = (instr & 0xFF) << 2;

    p->loadStore(true, true, true, false, false, rd, rn, offset);
}

void Thumbisa::LSL(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Word *rd = p->getVisibleRegister((Byte)(instr & 7));
    Word rs = *(p->getVisibleRegister((Byte)((instr >> 3) & 7)));
    Byte offset = (instr >> 6) & 0x1F;

    *rd = shift(rs, offset, SHIFT_LSL);
    p->bitwiseReturn(rd, true);  //MOVS
}

void Thumbisa::LSR(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Word *rd = p->getVisibleRegister((Byte)(instr & 7));
    Word rs = *(p->getVisibleRegister((Byte)((instr >> 3) & 7)));
    Byte offset = (instr >> 6) & 0x1F;

    *rd = shift(rs, offset, SHIFT_LSR);
    p->bitwiseReturn(rd, true);  //MOVS
}

void Thumbisa::MOV(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Word *rd = p->getVisibleRegister((Byte)(instr >> 8) & 7);

    *rd = instr & 0xFF;
    p->bitwiseReturn(rd, true);
}

void Thumbisa::MOVH(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Word *rd, *rs;
    Byte Hs = (instr >> 6) & 3;

    switch(Hs){
        case 1: //h2
            rd = p->getVisibleRegister((Byte)instr & 7);
            rs = p->getVisibleRegister((Byte)((instr >> 3) & 7) + 8);
            break;
        case 2: //h1
            rd = p->getVisibleRegister((Byte)(instr & 7) + 8);
            rs = p->getVisibleRegister((Byte)(instr >> 3) & 7);
        case 3: //h1&h2
            rd = p->getVisibleRegister((Byte)(instr & 7) + 8);
            rs = p->getVisibleRegister((Byte)((instr >> 3) & 7) + 8);
        default:
            p->unpredictable();
            return;
            break;
    }

    *rd = *rs;
    p->bitwiseReturn(rd, false);
}

void Thumbisa::POP(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    HalfWord list = instr & 0xFF;
    if((instr >> 8) & 1)
        list |= (1 << REG_PC);
    Word *sp = p->getVisibleRegister(REG_SP);

    p->blockDataTransfer(sp, list, true, false, true, false, true);
}

void Thumbisa::PUSH(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    HalfWord list = instr & 0xFF;
    if((instr >> 8) & 1)
        list |= (1 << REG_LR);
    Word *sp = p->getVisibleRegister(REG_SP);

    p->blockDataTransfer(sp, list, false, true, false, false, true);
}

void Thumbisa::STMIA(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Word *rb = p->getVisibleRegister((Byte)(instr >> 8) & 7);
    HalfWord list = instr & 0xFF;

    p->blockDataTransfer(rb, list, false, false, true, false, true);
}

void Thumbisa::STR(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Word *rb = p->getVisibleRegister((Byte)(instr >> 3) & 7);
    Word *rd = p->getVisibleRegister((Byte)instr & 7);
    Word offset;

    if(instr & (1<<13)){    //format 9 (immediate offset)
        offset = ((instr >> 6) & 0x1F) << 2;
    } else {                //format 7 (register offset)
        offset = *(p->getVisibleRegister((Byte)(instr >> 6) & 7));
    }

    p->loadStore(false, true, true, false, false, rd, rb, offset);
}

void Thumbisa::STRB(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Word *rb = p->getVisibleRegister((Byte)(instr >> 3) & 7);
    Word *rd = p->getVisibleRegister((Byte)instr & 7);
    Word offset;

    if(instr & (1<<13)){    //format 9 (immediate offset)
        offset = (instr >> 6) & 0x1F;
    } else {                //format 7 (register offset)
        offset = *(p->getVisibleRegister((Byte)(instr >> 6) & 7));
    }

    p->loadStore(false, true, true, true, false, rd, rb, offset);
}

void Thumbisa::STRH(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Word *rb = p->getVisibleRegister((Byte)(instr >> 3) & 7);
    Word *rd = p->getVisibleRegister((Byte)instr & 7);

    if(instr & (1<<13)){    //format 10 (immediate offset)
        Word offset = ((instr >> 6) & 0x1F) << 1;

        p->halfwordDataTransfer(rd, rb, NULL, offset, true, true, true, false, false, false);
    } else {                //format 8 (register offset)
        Word *ro = p->getVisibleRegister((Byte)(instr >> 6) & 7);

        p->halfwordDataTransfer(rd, rb, ro, 0, true, true, false, false, false, false);
    }
}

void Thumbisa::STRSP(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Word *rd = p->getVisibleRegister((Byte)(instr >> 8) & 7);
    Word *rn = p->getVisibleRegister(REG_SP);
    Word offset = (instr & 0xFF) << 2;

    p->loadStore(false, true, true, false, false, rd, rn, offset);
}

void Thumbisa::SUB(HalfWord instr){
	p->debugThumb(__FUNCTION__);

    Word *rd, *rs, offset;
    if(instr & (1<<13)){    //format 2
        rd = p->getVisibleRegister((Byte)instr & 7);
        rs = p->getVisibleRegister((Byte)(instr >> 3) & 7);
        offset = (instr >> 6) & 7;
        if(!(instr & (1<<10))){    //register offset
            offset = *(p->getVisibleRegister((Byte)offset));
        }
    } else {                //format 3
        rs = rd = p->getVisibleRegister((Byte)(instr >> 8) & 7);
        offset = instr & 0xFF;
    }

    p->dataPsum(*rs, offset, false, false, rd, true);
}

void Thumbisa::SWI(HalfWord instr){
	p->debugThumb(__FUNCTION__);
	
        p->softwareInterruptTrap();
}

void Thumbisa::UND(HalfWord instr){
	p->debugThumb(__FUNCTION__);
	
        p->undefinedTrap();
}

/*
 * ALU operations
 */

void Thumbisa::ADC_ALU(Word *rd, Word *rs){
	p->debugThumb(__FUNCTION__);

    p->dataPsum(*rd, *rs, true, true, rd, true);
}

void Thumbisa::AND_ALU(Word *rd, Word *rs){
	p->debugThumb(__FUNCTION__);

    *rd = *rd & *rs;
    p->bitwiseReturn(rd, true);
}

void Thumbisa::ASR_ALU(Word *rd, Word *rs){
	p->debugThumb(__FUNCTION__);

    *rd = shift(*rd, *rs, SHIFT_ASR);
    p->bitwiseReturn(rd, true);

}

void Thumbisa::BIC_ALU(Word *rd, Word *rs){
	p->debugThumb(__FUNCTION__);

    *rd = *rd & (INVERT_W(*rs));
    p->bitwiseReturn(rd, true);
}

void Thumbisa::CMN_ALU(Word *rd, Word *rs){
	p->debugThumb(__FUNCTION__);

    p->dataPsum(*rd, *rs, false, true, &(p->alu_tmp), true);
}

void Thumbisa::CMP_ALU(Word *rd, Word *rs){
	p->debugThumb(__FUNCTION__);

    p->dataPsum(*rd, *rs, false, false, &(p->alu_tmp), true);
}

void Thumbisa::EOR_ALU(Word *rd, Word *rs){
	p->debugThumb(__FUNCTION__);

    *rd = *rd ^ *rs;
    p->bitwiseReturn(rd, true);
}

void Thumbisa::LSL_ALU(Word *rd, Word *rs){
	p->debugThumb(__FUNCTION__);

    *rd = shift(*rd, *rs, SHIFT_LSL);
    p->bitwiseReturn(rd, true);
}

void Thumbisa::LSR_ALU(Word *rd, Word *rs){
	p->debugThumb(__FUNCTION__);

    *rd = shift(*rd, *rs, SHIFT_LSR);
    p->bitwiseReturn(rd, true);
}

void Thumbisa::MUL_ALU(Word *rd, Word *rs){
	p->debugThumb(__FUNCTION__);

    p->multiply(rd, rs, rd, NULL, false, false, true);
}

void Thumbisa::MVN_ALU(Word *rd, Word *rs){
	p->debugThumb(__FUNCTION__);

    *rd = INVERT_W(*rs);
    p->bitwiseReturn(rd, true);
}

void Thumbisa::NEG_ALU(Word *rd, Word *rs){
	p->debugThumb(__FUNCTION__);

    p->dataPsum((Word) 0, *rs, false, false, rd, true);
}

void Thumbisa::ORR_ALU(Word *rd, Word *rs){
	p->debugThumb(__FUNCTION__);

    *rd = *rd | *rs;
    p->bitwiseReturn(rd, true);
}

void Thumbisa::ROR_ALU(Word *rd, Word *rs){
	p->debugThumb(__FUNCTION__);

    *rd = shift(*rd, *rs, SHIFT_ROR);
    p->bitwiseReturn(rd, true);
}

void Thumbisa::SBC_ALU(Word *rd, Word *rs){
	p->debugThumb(__FUNCTION__);

    p->dataPsum(*rd, *rs, true, false, rd, true);
}

void Thumbisa::TST_ALU(Word *rd, Word *rs){
    p->debugThumb(__FUNCTION__);

    p->alu_tmp = *rd & *rs;
    p->bitwiseReturn(&(p->alu_tmp), true);
}

#endif //UARM_THUMBISA_CC
