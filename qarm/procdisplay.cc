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

#ifndef QARM_PROCDISPLAY_CC
#define QARM_PROCDISPLAY_CC

#include "qarm/procdisplay.h"

/*#include <iostream>
using namespace std;*/

procDisplay::procDisplay(QWidget *parent) :
    QWidget(parent)
{
    mainLayout = new QVBoxLayout;
    lowLayout = new QHBoxLayout;
    pipeL = new QGridLayout;
    cpuL = new QGridLayout;
    cp15L = new QGridLayout;
    infoL = new QGridLayout;
    lowLayout->addLayout(cp15L);
    lowLayout->addWidget(new QFLine(true));
    lowLayout->addLayout(infoL);
    mainLayout->addLayout(pipeL);
    mainLayout->addWidget(new QFLine(false));
    mainLayout->addLayout(cpuL);
    mainLayout->addWidget(new QFLine(false));
    mainLayout->addLayout(lowLayout);

    pipeline = new monoLabel*[PIPECOLS];
    cpuReg = new monoLabel**[CPUROWS];
    cp15Reg = new monoLabel**[CP15ROWS];
    infoReg = new monoLabel**[INFOROWS];

    for(int i = 0; i < PIPECOLS; i++)
        pipeline[i] = new monoLabel("");

    for(int i = 0; i < CPUROWS; i++){
        cpuReg[i] = new monoLabel*[CPUCOLS];
        for(int j = 0; j < CPUCOLS; j++)
            cpuReg[i][j] = new monoLabel("");
    }   

    for(int i = 0; i < CP15ROWS; i++){
        cp15Reg[i] = new monoLabel*[CP15COLS];
        for(int j = 0; j < CP15COLS; j++)
            cp15Reg[i][j] = new monoLabel("");
    }

    for(int i = 0; i < INFOROWS; i++){
        infoReg[i] = new monoLabel*[INFOCOLS];
        for(int j = 0; j < INFOCOLS; j++)
            infoReg[i][j] = new monoLabel("");
    }

    pipeline[0]->setText("Pipeline (E D F):");
    pipeline[4]->setText("Executing:");
    pipeline[5]->setAlignment(Qt::AlignLeft);

    cpuReg[0][0]->setText("CPU registers: ");
    cpuReg[1][0]->setText("r0(a1):");
    cpuReg[2][0]->setText("r1(a2):");
    cpuReg[3][0]->setText("r2(a3):");
    cpuReg[4][0]->setText("r3(a4):");
    cpuReg[5][0]->setText("r4(v1):");
    cpuReg[6][0]->setText("r5(v2):");
    cpuReg[7][0]->setText("r6(v3):");
    cpuReg[8][0]->setText("r7(v4):");
    cpuReg[9][0]->setText("r8(v5):");
    cpuReg[10][0]->setText("r9(v6):");
    cpuReg[11][0]->setText("r10(v7):");
    cpuReg[12][0]->setText("r11(v8):");
    cpuReg[13][0]->setText("r12(IP):");
    cpuReg[14][0]->setText("r13(SP):");
    cpuReg[15][0]->setText("r14(LR):");
    cpuReg[16][0]->setText("r15(PC):");
    cpuReg[17][0]->setText("CPSR:");
    cpuReg[18][0]->setText("SPSR:");

    for(int i = 0; i < CPUROWS; i++)
        cpuReg[i][0]->setAlignment(Qt::AlignRight);

    cpuReg[9][CPUCOLS-1]->setText(":r8");
    cpuReg[10][CPUCOLS-1]->setText(":r9(SB)");
    cpuReg[11][CPUCOLS-1]->setText(":r10(SL)");
    cpuReg[12][CPUCOLS-1]->setText(":r11(FP)");
    cpuReg[13][CPUCOLS-1]->setText(":r12(IP)");
    cpuReg[14][CPUCOLS-1]->setText(":r13(SP)");
    cpuReg[15][CPUCOLS-1]->setText(":r14(LR)");
    cpuReg[18][CPUCOLS-1]->setText(":SPSR");

    cpuReg[0][1]->setText("Usr/Sys");
    cpuReg[0][2]->setText("Svc");
    cpuReg[0][3]->setText("Abt");
    cpuReg[0][4]->setText("Undef");
    cpuReg[0][5]->setText("IRQ");
    cpuReg[0][6]->setText("FIQ");

    cp15Reg[0][0]->setText("CP15 registers:");
    cp15Reg[1][1]->setText("ID (r0):");
    cp15Reg[2][1]->setText("SCB (r1):");
    cp15Reg[3][1]->setText("PTE_Hi (r2):");
    cp15Reg[4][1]->setText("PTE_Low (r2):");
    cp15Reg[1][3]->setText("FA  (r6):");
    cp15Reg[2][3]->setText("TLBR  (r8):");
    cp15Reg[3][3]->setText("TLBI (r10):");
    cp15Reg[4][3]->setText("Cause (r15):");

    infoReg[0][0]->setText("BUS Info:");
    infoReg[1][1]->setText("TOD_Hi:");
    infoReg[2][1]->setText("TOD_Low:");
    infoReg[3][1]->setText("TIMER:");
    infoReg[4][1]->setText("Ram Size:");

    for(unsigned int i = 0; i < INFOROWS; i++){
        cp15Reg[i][1]->setAlignment(Qt::AlignRight);
        cp15Reg[i][3]->setAlignment(Qt::AlignRight);
        infoReg[i][1]->setAlignment(Qt::AlignRight);
    }

    setAccessibilityNames();

    reset();

    for(int i = 0; i < PIPECOLS; i++)
        pipeL->addWidget(pipeline[i],0,i);

    for(int i = 0; i < CPUROWS; i++)
        for(int j = 0; j < CPUCOLS; j++)
            cpuL->addWidget(cpuReg[i][j], i, j);

    for(int i = 0; i < CP15ROWS; i++)
        for(int j = 0; j < CP15COLS; j++)
            cp15L->addWidget(cp15Reg[i][j], i, j);

    for(int i = 0; i < INFOROWS; i++)
        for(int j = 0; j < INFOCOLS; j++)
            infoL->addWidget(infoReg[i][j], i, j);

    this->setLayout(mainLayout);
}

void procDisplay::setAccessibilityNames(){
    pipeline[1]->setAccessibleName("Pipeline Execute");
    pipeline[2]->setAccessibleName("Pipeline Decode");
    pipeline[3]->setAccessibleName("Pipeline Fetch");
    pipeline[5]->setAccessibleName("Executing Instruction");

    cpuReg[1][1]->setAccessibleName("Usr/Sys r0");
    cpuReg[1][1]->setAccessibleDescription("User and system mode r0 or a1");
    cpuReg[2][1]->setAccessibleName("Usr/Sys r1");
    cpuReg[2][1]->setAccessibleDescription("User and system mode r1 or a2");
    cpuReg[3][1]->setAccessibleName("Usr/Sys r2");
    cpuReg[3][1]->setAccessibleDescription("User and system mode r2 or a3");
    cpuReg[4][1]->setAccessibleName("Usr/Sys r3");
    cpuReg[4][1]->setAccessibleDescription("User and system mode r3 or a4");
    cpuReg[5][1]->setAccessibleName("Usr/Sys r4");
    cpuReg[5][1]->setAccessibleDescription("User and system mode r4 or v1");
    cpuReg[6][1]->setAccessibleName("Usr/Sys r5");
    cpuReg[6][1]->setAccessibleDescription("User and system mode r5 or v2");
    cpuReg[7][1]->setAccessibleName("Usr/Sys r6");
    cpuReg[7][1]->setAccessibleDescription("User and system mode r6 or v3");
    cpuReg[8][1]->setAccessibleName("Usr/Sys r7");
    cpuReg[8][1]->setAccessibleDescription("User and system mode r7 or v4");
    cpuReg[9][1]->setAccessibleName("Usr/Sys r8");
    cpuReg[9][1]->setAccessibleDescription("User and system mode r8 or v5");
    cpuReg[10][1]->setAccessibleName("Usr/Sys r9");
    cpuReg[10][1]->setAccessibleDescription("User and system mode r9 or v6 or SB");
    cpuReg[11][1]->setAccessibleName("Usr/Sys r10");
    cpuReg[11][1]->setAccessibleDescription("User and system mode r10 or v7 or SL");
    cpuReg[12][1]->setAccessibleName("Usr/Sys r11");
    cpuReg[12][1]->setAccessibleDescription("User and system mode r11 or v8 or FP");
    cpuReg[13][1]->setAccessibleName("Usr/Sys r12");
    cpuReg[13][1]->setAccessibleDescription("User and system mode r12 or IP");
    cpuReg[14][1]->setAccessibleName("Usr/Sys r13");
    cpuReg[14][1]->setAccessibleDescription("User and system mode r13 or SP");
    cpuReg[15][1]->setAccessibleName("Usr/Sys r14");
    cpuReg[15][1]->setAccessibleDescription("User and system mode r14 or LR");
    cpuReg[16][1]->setAccessibleName("Usr/Sys r15");
    cpuReg[16][1]->setAccessibleDescription("User and system mode r15 or PC");
    cpuReg[17][1]->setAccessibleName("CPSR");

    cpuReg[14][2]->setAccessibleName("Svc r13");
    cpuReg[14][2]->setAccessibleDescription("Supervisor mode r13 or SP");
    cpuReg[15][2]->setAccessibleName("Svc r14");
    cpuReg[15][2]->setAccessibleDescription("Supervisor mode r14 or LR");
    cpuReg[18][2]->setAccessibleName("Svc SPSR");
    cpuReg[18][2]->setAccessibleDescription("Supervisor mode SPSR");

    cpuReg[14][3]->setAccessibleName("Abt r13");
    cpuReg[14][3]->setAccessibleDescription("Abort mode r13 or SP");
    cpuReg[15][3]->setAccessibleName("Abt r14");
    cpuReg[15][3]->setAccessibleDescription("Abort mode r14 or LR");
    cpuReg[18][3]->setAccessibleName("Abt SPSR");
    cpuReg[18][3]->setAccessibleDescription("Abort mode SPSR");

    cpuReg[14][4]->setAccessibleName("Undef r13");
    cpuReg[14][4]->setAccessibleDescription("Undefined mode r13 or SP");
    cpuReg[15][4]->setAccessibleName("Undef r14");
    cpuReg[15][4]->setAccessibleDescription("Undefined mode r14 or LR");
    cpuReg[18][4]->setAccessibleName("Undef SPSR");
    cpuReg[18][4]->setAccessibleDescription("Undefined mode SPSR");

    cpuReg[14][5]->setAccessibleName("Irq r13");
    cpuReg[14][5]->setAccessibleDescription("Interrupt mode r13 or SP");
    cpuReg[15][5]->setAccessibleName("Irq r14");
    cpuReg[15][5]->setAccessibleDescription("Interrupt mode r14 or LR");
    cpuReg[18][5]->setAccessibleName("Irq SPSR");
    cpuReg[18][5]->setAccessibleDescription("Interrupt mode SPSR");

    cpuReg[9][6]->setAccessibleName("Fiq r8");
    cpuReg[9][6]->setAccessibleDescription("Fast Interrupt mode r8 or v5");
    cpuReg[10][6]->setAccessibleName("Fiq r9");
    cpuReg[10][6]->setAccessibleDescription("Fast Interrupt mode r9 or v6 or SB");
    cpuReg[11][6]->setAccessibleName("Fiq r10");
    cpuReg[11][6]->setAccessibleDescription("Fast Interrupt mode r10 or v7 or SL");
    cpuReg[12][6]->setAccessibleName("Fiq r11");
    cpuReg[12][6]->setAccessibleDescription("Fast Interrupt mode r11 or v8 or FP");
    cpuReg[13][6]->setAccessibleName("Fiq r12");
    cpuReg[13][6]->setAccessibleDescription("Fast Interrupt mode r12 or IP");
    cpuReg[14][6]->setAccessibleName("Fiq r13");
    cpuReg[14][6]->setAccessibleDescription("Fast Interrupt mode r13 or SP");
    cpuReg[15][6]->setAccessibleName("Fiq r14");
    cpuReg[15][6]->setAccessibleDescription("Fast Interrupt mode r14 or LR");
    cpuReg[16][6]->setAccessibleName("Fiq r15");
    cpuReg[16][6]->setAccessibleDescription("Fast Interrupt mode r15 or PC");
    cpuReg[18][6]->setAccessibleName("Fiq SPSR");
    cpuReg[18][6]->setAccessibleDescription("Fast Interrupt mode SPSR");

    cp15Reg[1][2]->setAccessibleName("CP15 ID");
    cp15Reg[1][2]->setAccessibleDescription("CP15 register r0 or ID");
    cp15Reg[2][2]->setAccessibleName("CP15 SCB");
    cp15Reg[2][2]->setAccessibleDescription("CP15 register r1 or SCB");
    cp15Reg[3][2]->setAccessibleName("CP15 PTE_Hi");
    cp15Reg[3][2]->setAccessibleDescription("CP15 register r2 or PTE, High part");
    cp15Reg[4][2]->setAccessibleName("CP15 PTE_Low");
    cp15Reg[4][2]->setAccessibleDescription("CP15 register r2 or PTE, Low part");
    cp15Reg[1][4]->setAccessibleName("CP15 FA");
    cp15Reg[1][4]->setAccessibleDescription("CP15 register r6 or FA");
    cp15Reg[2][4]->setAccessibleName("CP15 TLBR");
    cp15Reg[2][4]->setAccessibleDescription("CP15 register r8 or TLBR");
    cp15Reg[3][4]->setAccessibleName("CP15 TLBI");
    cp15Reg[3][4]->setAccessibleDescription("CP15 register r10 or TLBI");
    cp15Reg[4][4]->setAccessibleName("CP15 Cause");
    cp15Reg[4][4]->setAccessibleDescription("CP15 register r15 or Cause");

    infoReg[1][2]->setAccessibleName("TOD_Hi");
    infoReg[2][2]->setAccessibleName("TOD_Low");
    infoReg[3][2]->setAccessibleName("TIMER");
    infoReg[4][2]->setAccessibleName("Ram Size");
}

void procDisplay::reset(){
    pipeline[1]->setText(convertHex(0));
    pipeline[2]->setText(convertHex(0));
    pipeline[3]->setText(convertHex(0));
    pipeline[5]->setText("NOP");

    for(int i = 0; i < 17; i++)
        cpuReg[i+1][1]->setText(convertHex(0));
    for(int i = 0; i < 7; i++)
        cpuReg[i+9][6]->setText(convertHex(0));
    cpuReg[18][6]->setText(convertHex(0));
    cpuReg[14][2]->setText(convertHex(0));
    cpuReg[15][2]->setText(convertHex(0));
    cpuReg[18][2]->setText(convertHex(0));
    cpuReg[14][3]->setText(convertHex(0));
    cpuReg[15][3]->setText(convertHex(0));
    cpuReg[18][3]->setText(convertHex(0));
    cpuReg[14][5]->setText(convertHex(0));
    cpuReg[15][5]->setText(convertHex(0));
    cpuReg[18][5]->setText(convertHex(0));
    cpuReg[14][4]->setText(convertHex(0));
    cpuReg[15][4]->setText(convertHex(0));
    cpuReg[18][4]->setText(convertHex(0));

    cp15Reg[1][2]->setText(convertHex(0));
    cp15Reg[2][2]->setText(convertHex(0));
    cp15Reg[3][2]->setText(convertHex(0));
    cp15Reg[4][2]->setText(convertHex(0));
    cp15Reg[1][4]->setText(convertHex(0));
    cp15Reg[2][4]->setText(convertHex(0));
    cp15Reg[3][4]->setText(convertHex(0));
    cp15Reg[4][4]->setText(convertHex(0));

    infoReg[1][2]->setText(convertHex(0));
    infoReg[2][2]->setText(convertHex(0));
    infoReg[3][2]->setText(convertHex(0));
    infoReg[4][2]->setText(convertHex(MC_Holder::getInstance()->getConfig()->getRamSize()*BYTES_PER_FRAME));
}

void procDisplay::updateVals(Word *cpu, Word *cp15, Word *ppln, Word todH, Word todL, Word timer, QString ass){
    pipeline[1]->setText(convertHex(ppln[2]));
    pipeline[2]->setText(convertHex(ppln[1]));
    pipeline[3]->setText(convertHex(ppln[0]));
    pipeline[5]->setText(ass);

    for(int i = 0; i < 17; i++)
        cpuReg[i+1][1]->setText(convertHex(cpu[i]));
    for(int i = 0; i < 7; i++)
        cpuReg[i+9][6]->setText(convertHex(cpu[i+17]));
    cpuReg[18][6]->setText(convertHex(cpu[24]));
    cpuReg[14][2]->setText(convertHex(cpu[25]));
    cpuReg[15][2]->setText(convertHex(cpu[26]));
    cpuReg[18][2]->setText(convertHex(cpu[27]));
    cpuReg[14][3]->setText(convertHex(cpu[28]));
    cpuReg[15][3]->setText(convertHex(cpu[29]));
    cpuReg[18][3]->setText(convertHex(cpu[30]));
    cpuReg[14][5]->setText(convertHex(cpu[31]));
    cpuReg[15][5]->setText(convertHex(cpu[32]));
    cpuReg[18][5]->setText(convertHex(cpu[33]));
    cpuReg[14][4]->setText(convertHex(cpu[34]));
    cpuReg[15][4]->setText(convertHex(cpu[35]));
    cpuReg[18][4]->setText(convertHex(cpu[36]));

    cp15Reg[1][2]->setText(convertHex(cp15[CP15_REG0_IDC]));
    cp15Reg[2][2]->setText(convertHex(cp15[CP15_REG1_SCB]));
    cp15Reg[3][2]->setText(convertHex(cp15[CP15_REG2_EntryHi]));
    cp15Reg[4][2]->setText(convertHex(cp15[CP15_REG2_EntryLo]));
    cp15Reg[1][4]->setText(convertHex(cp15[CP15_REG6_FA]));
    cp15Reg[2][4]->setText(convertHex(cp15[CP15_REG8_TLBR]));
    cp15Reg[3][4]->setText(convertHex(cp15[CP15_REG10_TLBI]));
    cp15Reg[4][4]->setText(convertHex(cp15[CP15_REG15_CAUSE]));

    infoReg[1][2]->setText(convertHex(todH));
    infoReg[2][2]->setText(convertHex(todL));
    infoReg[3][2]->setText(convertHex(timer));
}

QString procDisplay::convertHex(Word val){
    QString ret = "0x";
    Word mask = 0xF0000000;
    int count = 28;
    int ref = (val & mask) >> count;
    while(mask > 0){
        switch(ref){
        case 0: ret += "0"; break;
        case 1: ret += "1"; break;
        case 2: ret += "2"; break;
        case 3: ret += "3"; break;
        case 4: ret += "4"; break;
        case 5: ret += "5"; break;
        case 6: ret += "6"; break;
        case 7: ret += "7"; break;
        case 8: ret += "8"; break;
        case 9: ret += "9"; break;
        case 10: ret += "A"; break;
        case 11: ret += "B"; break;
        case 12: ret += "C"; break;
        case 13: ret += "D"; break;
        case 14: ret += "E"; break;
        case 15: ret += "F"; break;
        }
        mask >>= 4;
        count -= 4;
        ref = (val & mask) >> count;
    }
    return ret;
}

#endif //QARM_PROCDISPLAY_CC
