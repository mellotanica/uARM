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

#define convertHex(val) { "0x" + QString("%1").arg(val, 8, 16, QChar('0')).toUpper() }

#define BUSHEADER "Bus registers and Pipeline contents"
#define USRHEADER "USR/SYS registers contents"
#define SVCHEADER "SVC registers contents"
#define ABTHEADER "ABT registers contents"
#define UNDHEADER "UND registers contents"
#define IRQHEADER "IRQ registers contents"
#define FIQHEADER "FIQ registers contents"
#define CP15HEADER "CP15 registers contents"

procDisplay::procDisplay(QWidget *parent) :
    QWidget(parent)
{
    if(MC_Holder::getInstance()->getConfig()->getAccessibleMode()){
        this->setLayout(createAccessibleLayout());
    } else {
        this->setLayout(createStandardLayout());
    }
}

QVBoxLayout *procDisplay::createStandardLayout(){
    QVBoxLayout *mainLayout = new QVBoxLayout();
    QHBoxLayout *lowLayout = new QHBoxLayout;
    QGridLayout *pipeL = new QGridLayout;
    QGridLayout *cpuL = new QGridLayout;
    QGridLayout *cp15L = new QGridLayout;
    QGridLayout *infoL = new QGridLayout;
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
        pipeline[i] = new monoLabel("", this);

    for(int i = 0; i < CPUROWS; i++){
        cpuReg[i] = new monoLabel*[CPUCOLS];
        for(int j = 0; j < CPUCOLS; j++)
            cpuReg[i][j] = new monoLabel("", this);
    }

    for(int i = 0; i < CP15ROWS; i++){
        cp15Reg[i] = new monoLabel*[CP15COLS];
        for(int j = 0; j < CP15COLS; j++)
            cp15Reg[i][j] = new monoLabel("", this);
    }

    for(int i = 0; i < INFOROWS; i++){
        infoReg[i] = new monoLabel*[INFOCOLS];
        for(int j = 0; j < INFOCOLS; j++)
            infoReg[i][j] = new monoLabel("", this);
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

    setAccessibilityLabels();

    resetLabels(0x22222222);

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

    return mainLayout;
}

void procDisplay::setAccessibilityLabels(){
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

void procDisplay::resetLabels(int val){
    pipeline[1]->setText(convertHex(val));
    pipeline[2]->setText(convertHex(val));
    pipeline[3]->setText(convertHex(val));
    pipeline[5]->setText("NOP");

    for(int i = 0; i < 17; i++)
        cpuReg[i+1][1]->setText(convertHex(val));
    for(int i = 0; i < 7; i++)
        cpuReg[i+9][6]->setText(convertHex(val));
    cpuReg[18][6]->setText(convertHex(val));
    cpuReg[14][2]->setText(convertHex(val));
    cpuReg[15][2]->setText(convertHex(val));
    cpuReg[18][2]->setText(convertHex(val));
    cpuReg[14][3]->setText(convertHex(val));
    cpuReg[15][3]->setText(convertHex(val));
    cpuReg[18][3]->setText(convertHex(val));
    cpuReg[14][5]->setText(convertHex(val));
    cpuReg[15][5]->setText(convertHex(val));
    cpuReg[18][5]->setText(convertHex(val));
    cpuReg[14][4]->setText(convertHex(val));
    cpuReg[15][4]->setText(convertHex(val));
    cpuReg[18][4]->setText(convertHex(val));

    cp15Reg[1][2]->setText(convertHex(val));
    cp15Reg[2][2]->setText(convertHex(val));
    cp15Reg[3][2]->setText(convertHex(val));
    cp15Reg[4][2]->setText(convertHex(val));
    cp15Reg[1][4]->setText(convertHex(val));
    cp15Reg[2][4]->setText(convertHex(val));
    cp15Reg[3][4]->setText(convertHex(val));
    cp15Reg[4][4]->setText(convertHex(val));

    infoReg[1][2]->setText(convertHex(val));
    infoReg[2][2]->setText(convertHex(val));
    infoReg[3][2]->setText(convertHex(val));
    infoReg[4][2]->setText(convertHex(val));
    memsize = MC_Holder::getInstance()->getConfig()->getRamSize()*BYTES_PER_FRAME;
}

void procDisplay::updateLabels(Word *cpu, Word *cp15, Word *ppln, Word todH, Word todL, Word timer, QString ass){
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
    infoReg[4][2]->setText(convertHex(memsize));
}

QHBoxLayout *procDisplay::createAccessibleLayout(){
    QHBoxLayout *mainLayout = new QHBoxLayout();

    busTA = new QTextEdit(this);
    usrTA = new QTextEdit(this);
    srvTA = new QTextEdit(this);
    abtTA = new QTextEdit(this);
    undTA = new QTextEdit(this);
    irqTA = new QTextEdit(this);
    fiqTA = new QTextEdit(this);
    cp15TA = new QTextEdit(this);

    busTA->setReadOnly(true);
    usrTA->setReadOnly(true);
    srvTA->setReadOnly(true);
    abtTA->setReadOnly(true);
    undTA->setReadOnly(true);
    irqTA->setReadOnly(true);
    fiqTA->setReadOnly(true);
    cp15TA->setReadOnly(true);

    busTA->setAccessibleName(BUSHEADER);
    usrTA->setAccessibleName(USRHEADER);
    srvTA->setAccessibleName(SVCHEADER);
    abtTA->setAccessibleName(ABTHEADER);
    undTA->setAccessibleName(UNDHEADER);
    irqTA->setAccessibleName(IRQHEADER);
    fiqTA->setAccessibleName(FIQHEADER);
    cp15TA->setAccessibleName(CP15HEADER);

    mainLayout->addWidget(busTA);
    mainLayout->addWidget(usrTA);
    mainLayout->addWidget(srvTA);
    mainLayout->addWidget(abtTA);
    mainLayout->addWidget(undTA);
    mainLayout->addWidget(irqTA);
    mainLayout->addWidget(fiqTA);
    mainLayout->addWidget(cp15TA);

    resetTexts();

    return mainLayout;
}

void procDisplay::resetTexts(){
    busTA->setText(QString("Bus Resgisters contents:\n")+
                   "Tod_Hi: "+QString(convertHex(0))+"\n"+
                   "Tod_Lo: "+QString(convertHex(0))+"\n"+
                   "Timer: "+QString(convertHex(0))+"\n"+
                   "Ram Size: "+QString(convertHex(0))+"\n"+
                   "Pipeline contents:\n"+
                   "Fetch: "+QString(convertHex(0))+"\n"+
                   "Decode: "+QString(convertHex(0))+"\n"+
                   "Execute: "+QString(convertHex(0))+"\n"+
                   "Execute (ARM): ");

    usrTA->setText(QString(USRHEADER)+
                   "r0(a1): "+QString(convertHex(0))+"\n"+
                   "r1(a2): "+QString(convertHex(0))+"\n"+
                   "r2(a3): "+QString(convertHex(0))+"\n"+
                   "r3(a4): "+QString(convertHex(0))+"\n"+
                   "r4(v1): "+QString(convertHex(0))+"\n"+
                   "r5(v2): "+QString(convertHex(0))+"\n"+
                   "r6(v3): "+QString(convertHex(0))+"\n"+
                   "r7(v4): "+QString(convertHex(0))+"\n"+
                   "r8(v5): "+QString(convertHex(0))+"\n"+
                   "r9(SB): "+QString(convertHex(0))+"\n"+
                   "r10(SL): "+QString(convertHex(0))+"\n"+
                   "r11(FP): "+QString(convertHex(0))+"\n"+
                   "r12(IP): "+QString(convertHex(0))+"\n"+
                   "r13(SP): "+QString(convertHex(0))+"\n"+
                   "r14(LR): "+QString(convertHex(0))+"\n"+
                   "r15(PC): "+QString(convertHex(0))+"\n"+
                   "CPSR: "+QString(convertHex(0)));

    srvTA->setText(QString(SVCHEADER)+
                   "r0(a1): "+QString(convertHex(0))+"\n"+
                   "r1(a2): "+QString(convertHex(0))+"\n"+
                   "r2(a3): "+QString(convertHex(0))+"\n"+
                   "r3(a4): "+QString(convertHex(0))+"\n"+
                   "r4(v1): "+QString(convertHex(0))+"\n"+
                   "r5(v2): "+QString(convertHex(0))+"\n"+
                   "r6(v3): "+QString(convertHex(0))+"\n"+
                   "r7(v4): "+QString(convertHex(0))+"\n"+
                   "r8(v5): "+QString(convertHex(0))+"\n"+
                   "r9(SB): "+QString(convertHex(0))+"\n"+
                   "r10(SL): "+QString(convertHex(0))+"\n"+
                   "r11(FP): "+QString(convertHex(0))+"\n"+
                   "r12(IP): "+QString(convertHex(0))+"\n"+
                   "r13(SP): "+QString(convertHex(0))+"\n"+
                   "r14(LR): "+QString(convertHex(0))+"\n"+
                   "r15(PC): "+QString(convertHex(0))+"\n"+
                   "CPSR: "+QString(convertHex(0))+"\n"+
                   "SPSR: "+QString(convertHex(0)));

    abtTA->setText(QString(ABTHEADER)+
                   "r0(a1): "+QString(convertHex(0))+"\n"+
                   "r1(a2): "+QString(convertHex(0))+"\n"+
                   "r2(a3): "+QString(convertHex(0))+"\n"+
                   "r3(a4): "+QString(convertHex(0))+"\n"+
                   "r4(v1): "+QString(convertHex(0))+"\n"+
                   "r5(v2): "+QString(convertHex(0))+"\n"+
                   "r6(v3): "+QString(convertHex(0))+"\n"+
                   "r7(v4): "+QString(convertHex(0))+"\n"+
                   "r8(v5): "+QString(convertHex(0))+"\n"+
                   "r9(SB): "+QString(convertHex(0))+"\n"+
                   "r10(SL): "+QString(convertHex(0))+"\n"+
                   "r11(FP): "+QString(convertHex(0))+"\n"+
                   "r12(IP): "+QString(convertHex(0))+"\n"+
                   "r13(SP): "+QString(convertHex(0))+"\n"+
                   "r14(LR): "+QString(convertHex(0))+"\n"+
                   "r15(PC): "+QString(convertHex(0))+"\n"+
                   "CPSR: "+QString(convertHex(0))+"\n"+
                   "SPSR: "+QString(convertHex(0)));

    undTA->setText(QString(UNDHEADER)+
                   "r0(a1): "+QString(convertHex(0))+"\n"+
                   "r1(a2): "+QString(convertHex(0))+"\n"+
                   "r2(a3): "+QString(convertHex(0))+"\n"+
                   "r3(a4): "+QString(convertHex(0))+"\n"+
                   "r4(v1): "+QString(convertHex(0))+"\n"+
                   "r5(v2): "+QString(convertHex(0))+"\n"+
                   "r6(v3): "+QString(convertHex(0))+"\n"+
                   "r7(v4): "+QString(convertHex(0))+"\n"+
                   "r8(v5): "+QString(convertHex(0))+"\n"+
                   "r9(SB): "+QString(convertHex(0))+"\n"+
                   "r10(SL): "+QString(convertHex(0))+"\n"+
                   "r11(FP): "+QString(convertHex(0))+"\n"+
                   "r12(IP): "+QString(convertHex(0))+"\n"+
                   "r13(SP): "+QString(convertHex(0))+"\n"+
                   "r14(LR): "+QString(convertHex(0))+"\n"+
                   "r15(PC): "+QString(convertHex(0))+"\n"+
                   "CPSR: "+QString(convertHex(0))+"\n"+
                   "SPSR: "+QString(convertHex(0)));

    irqTA->setText(QString(IRQHEADER)+
                   "r0(a1): "+QString(convertHex(0))+"\n"+
                   "r1(a2): "+QString(convertHex(0))+"\n"+
                   "r2(a3): "+QString(convertHex(0))+"\n"+
                   "r3(a4): "+QString(convertHex(0))+"\n"+
                   "r4(v1): "+QString(convertHex(0))+"\n"+
                   "r5(v2): "+QString(convertHex(0))+"\n"+
                   "r6(v3): "+QString(convertHex(0))+"\n"+
                   "r7(v4): "+QString(convertHex(0))+"\n"+
                   "r8(v5): "+QString(convertHex(0))+"\n"+
                   "r9(SB): "+QString(convertHex(0))+"\n"+
                   "r10(SL): "+QString(convertHex(0))+"\n"+
                   "r11(FP): "+QString(convertHex(0))+"\n"+
                   "r12(IP): "+QString(convertHex(0))+"\n"+
                   "r13(SP): "+QString(convertHex(0))+"\n"+
                   "r14(LR): "+QString(convertHex(0))+"\n"+
                   "r15(PC): "+QString(convertHex(0))+"\n"+
                   "CPSR: "+QString(convertHex(0))+"\n"+
                   "SPSR: "+QString(convertHex(0)));

    fiqTA->setText(QString(FIQHEADER)+
                   "r0(a1): "+QString(convertHex(0))+"\n"+
                   "r1(a2): "+QString(convertHex(0))+"\n"+
                   "r2(a3): "+QString(convertHex(0))+"\n"+
                   "r3(a4): "+QString(convertHex(0))+"\n"+
                   "r4(v1): "+QString(convertHex(0))+"\n"+
                   "r5(v2): "+QString(convertHex(0))+"\n"+
                   "r6(v3): "+QString(convertHex(0))+"\n"+
                   "r7(v4): "+QString(convertHex(0))+"\n"+
                   "r8(v5): "+QString(convertHex(0))+"\n"+
                   "r9(SB): "+QString(convertHex(0))+"\n"+
                   "r10(SL): "+QString(convertHex(0))+"\n"+
                   "r11(FP): "+QString(convertHex(0))+"\n"+
                   "r12(IP): "+QString(convertHex(0))+"\n"+
                   "r13(SP): "+QString(convertHex(0))+"\n"+
                   "r14(LR): "+QString(convertHex(0))+"\n"+
                   "r15(PC): "+QString(convertHex(0))+"\n"+
                   "CPSR: "+QString(convertHex(0))+"\n"+
                   "SPSR: "+QString(convertHex(0)));

    cp15TA->setText(QString(CP15HEADER)+
                   "ID(r0): "+QString(convertHex(0))+"\n"+
                   "SCB(r1): "+QString(convertHex(0))+"\n"+
                   "PTE_Hi(r2): "+QString(convertHex(0))+"\n"+
                   "PTE_Low(r2): "+QString(convertHex(0))+"\n"+
                   "FA(r6): "+QString(convertHex(0))+"\n"+
                   "TLBR(r8): "+QString(convertHex(0))+"\n"+
                   "TLBI(r10): "+QString(convertHex(0))+"\n"+
                   "CAUSE(r15): "+QString(convertHex(0)));

    memsize = MC_Holder::getInstance()->getConfig()->getRamSize()*BYTES_PER_FRAME;
}

void procDisplay::updateTexts(Word *cpu, Word *cp15, Word *ppln, Word todH, Word todL, Word timer, QString ass){
    busTA->setText(QString("Bus Resgisters contents:\n")+
                   "Tod_Hi: "+QString(convertHex(todH))+"\n"+
                   "Tod_Lo: "+QString(convertHex(todL))+"\n"+
                   "Timer: "+QString(convertHex(timer))+"\n"+
                   "Ram Size: "+QString(convertHex(memsize))+"\n"+
                   "Pipeline contents:\n"+
                   "Fetch: "+QString(convertHex(ppln[PIPELINE_FETCH]))+"\n"+
                   "Decode: "+QString(convertHex(ppln[PIPELINE_DECODE]))+"\n"+
                   "Execute: "+QString(convertHex(ppln[PIPELINE_EXECUTE]))+"\n"+
                   "Execute (ARM): "+ass);

    usrTA->setText(QString(USRHEADER)+
                   "r0(a1): "+QString(convertHex(cpu[0]))+"\n"+
                   "r1(a2): "+QString(convertHex(cpu[1]))+"\n"+
                   "r2(a3): "+QString(convertHex(cpu[2]))+"\n"+
                   "r3(a4): "+QString(convertHex(cpu[3]))+"\n"+
                   "r4(v1): "+QString(convertHex(cpu[4]))+"\n"+
                   "r5(v2): "+QString(convertHex(cpu[5]))+"\n"+
                   "r6(v3): "+QString(convertHex(cpu[6]))+"\n"+
                   "r7(v4): "+QString(convertHex(cpu[7]))+"\n"+
                   "r8(v5): "+QString(convertHex(cpu[8]))+"\n"+
                   "r9(SB): "+QString(convertHex(cpu[9]))+"\n"+
                   "r10(SL): "+QString(convertHex(cpu[10]))+"\n"+
                   "r11(FP): "+QString(convertHex(cpu[11]))+"\n"+
                   "r12(IP): "+QString(convertHex(cpu[12]))+"\n"+
                   "r13(SP): "+QString(convertHex(cpu[13]))+"\n"+
                   "r14(LR): "+QString(convertHex(cpu[14]))+"\n"+
                   "r15(PC): "+QString(convertHex(cpu[15]))+"\n"+
                   "CPSR: "+QString(convertHex(cpu[REG_CPSR])));

    srvTA->setText(QString(SVCHEADER)+
                   "r0(a1): "+QString(convertHex(cpu[0]))+"\n"+
                   "r1(a2): "+QString(convertHex(cpu[1]))+"\n"+
                   "r2(a3): "+QString(convertHex(cpu[2]))+"\n"+
                   "r3(a4): "+QString(convertHex(cpu[3]))+"\n"+
                   "r4(v1): "+QString(convertHex(cpu[4]))+"\n"+
                   "r5(v2): "+QString(convertHex(cpu[5]))+"\n"+
                   "r6(v3): "+QString(convertHex(cpu[6]))+"\n"+
                   "r7(v4): "+QString(convertHex(cpu[7]))+"\n"+
                   "r8(v5): "+QString(convertHex(cpu[8]))+"\n"+
                   "r9(SB): "+QString(convertHex(cpu[9]))+"\n"+
                   "r10(SL): "+QString(convertHex(cpu[10]))+"\n"+
                   "r11(FP): "+QString(convertHex(cpu[11]))+"\n"+
                   "r12(IP): "+QString(convertHex(cpu[12]))+"\n"+
                   "r13(SP): "+QString(convertHex(cpu[REG_SVC_BASE]))+"\n"+
                   "r14(LR): "+QString(convertHex(cpu[REG_SVC_BASE+1]))+"\n"+
                   "r15(PC): "+QString(convertHex(cpu[15]))+"\n"+
                   "CPSR: "+QString(convertHex(cpu[REG_CPSR]))+"\n"+
                   "SPSR: "+QString(convertHex(cpu[REG_SPSR_SVC])));

    abtTA->setText(QString(ABTHEADER)+
                   "r0(a1): "+QString(convertHex(cpu[0]))+"\n"+
                   "r1(a2): "+QString(convertHex(cpu[1]))+"\n"+
                   "r2(a3): "+QString(convertHex(cpu[2]))+"\n"+
                   "r3(a4): "+QString(convertHex(cpu[3]))+"\n"+
                   "r4(v1): "+QString(convertHex(cpu[4]))+"\n"+
                   "r5(v2): "+QString(convertHex(cpu[5]))+"\n"+
                   "r6(v3): "+QString(convertHex(cpu[6]))+"\n"+
                   "r7(v4): "+QString(convertHex(cpu[7]))+"\n"+
                   "r8(v5): "+QString(convertHex(cpu[8]))+"\n"+
                   "r9(SB): "+QString(convertHex(cpu[9]))+"\n"+
                   "r10(SL): "+QString(convertHex(cpu[10]))+"\n"+
                   "r11(FP): "+QString(convertHex(cpu[11]))+"\n"+
                   "r12(IP): "+QString(convertHex(cpu[12]))+"\n"+
                   "r13(SP): "+QString(convertHex(cpu[REG_ABT_BASE]))+"\n"+
                   "r14(LR): "+QString(convertHex(cpu[REG_ABT_BASE+1]))+"\n"+
                   "r15(PC): "+QString(convertHex(cpu[15]))+"\n"+
                   "CPSR: "+QString(convertHex(cpu[REG_CPSR]))+"\n"+
                   "SPSR: "+QString(convertHex(cpu[REG_SPSR_ABT])));

    undTA->setText(QString(UNDHEADER)+
                   "r0(a1): "+QString(convertHex(cpu[0]))+"\n"+
                   "r1(a2): "+QString(convertHex(cpu[1]))+"\n"+
                   "r2(a3): "+QString(convertHex(cpu[2]))+"\n"+
                   "r3(a4): "+QString(convertHex(cpu[3]))+"\n"+
                   "r4(v1): "+QString(convertHex(cpu[4]))+"\n"+
                   "r5(v2): "+QString(convertHex(cpu[5]))+"\n"+
                   "r6(v3): "+QString(convertHex(cpu[6]))+"\n"+
                   "r7(v4): "+QString(convertHex(cpu[7]))+"\n"+
                   "r8(v5): "+QString(convertHex(cpu[8]))+"\n"+
                   "r9(SB): "+QString(convertHex(cpu[9]))+"\n"+
                   "r10(SL): "+QString(convertHex(cpu[10]))+"\n"+
                   "r11(FP): "+QString(convertHex(cpu[11]))+"\n"+
                   "r12(IP): "+QString(convertHex(cpu[12]))+"\n"+
                   "r13(SP): "+QString(convertHex(cpu[REG_UNDEF_BASE]))+"\n"+
                   "r14(LR): "+QString(convertHex(cpu[REG_UNDEF_BASE+1]))+"\n"+
                   "r15(PC): "+QString(convertHex(cpu[15]))+"\n"+
                   "CPSR: "+QString(convertHex(cpu[REG_CPSR]))+"\n"+
                   "SPSR: "+QString(convertHex(cpu[REG_SPSR_UND])));

    irqTA->setText(QString(IRQHEADER)+
                   "r0(a1): "+QString(convertHex(cpu[0]))+"\n"+
                   "r1(a2): "+QString(convertHex(cpu[1]))+"\n"+
                   "r2(a3): "+QString(convertHex(cpu[2]))+"\n"+
                   "r3(a4): "+QString(convertHex(cpu[3]))+"\n"+
                   "r4(v1): "+QString(convertHex(cpu[4]))+"\n"+
                   "r5(v2): "+QString(convertHex(cpu[5]))+"\n"+
                   "r6(v3): "+QString(convertHex(cpu[6]))+"\n"+
                   "r7(v4): "+QString(convertHex(cpu[7]))+"\n"+
                   "r8(v5): "+QString(convertHex(cpu[8]))+"\n"+
                   "r9(SB): "+QString(convertHex(cpu[9]))+"\n"+
                   "r10(SL): "+QString(convertHex(cpu[10]))+"\n"+
                   "r11(FP): "+QString(convertHex(cpu[11]))+"\n"+
                   "r12(IP): "+QString(convertHex(cpu[12]))+"\n"+
                   "r13(SP): "+QString(convertHex(cpu[REG_IRQ_BASE]))+"\n"+
                   "r14(LR): "+QString(convertHex(cpu[REG_IRQ_BASE+1]))+"\n"+
                   "r15(PC): "+QString(convertHex(cpu[15]))+"\n"+
                   "CPSR: "+QString(convertHex(cpu[REG_CPSR]))+"\n"+
                   "SPSR: "+QString(convertHex(cpu[REG_SPSR_IRQ])));

    fiqTA->setText(QString(FIQHEADER)+
                   "r0(a1): "+QString(convertHex(cpu[0]))+"\n"+
                   "r1(a2): "+QString(convertHex(cpu[1]))+"\n"+
                   "r2(a3): "+QString(convertHex(cpu[2]))+"\n"+
                   "r3(a4): "+QString(convertHex(cpu[3]))+"\n"+
                   "r4(v1): "+QString(convertHex(cpu[4]))+"\n"+
                   "r5(v2): "+QString(convertHex(cpu[5]))+"\n"+
                   "r6(v3): "+QString(convertHex(cpu[6]))+"\n"+
                   "r7(v4): "+QString(convertHex(cpu[7]))+"\n"+
                   "r8(v5): "+QString(convertHex(cpu[REG_FIQ_BASE]))+"\n"+
                   "r9(SB): "+QString(convertHex(cpu[REG_FIQ_BASE+1]))+"\n"+
                   "r10(SL): "+QString(convertHex(cpu[REG_FIQ_BASE+2]))+"\n"+
                   "r11(FP): "+QString(convertHex(cpu[REG_FIQ_BASE+3]))+"\n"+
                   "r12(IP): "+QString(convertHex(cpu[REG_FIQ_BASE+4]))+"\n"+
                   "r13(SP): "+QString(convertHex(cpu[REG_SVC_BASE+5]))+"\n"+
                   "r14(LR): "+QString(convertHex(cpu[REG_SVC_BASE+6]))+"\n"+
                   "r15(PC): "+QString(convertHex(cpu[15]))+"\n"+
                   "CPSR: "+QString(convertHex(cpu[REG_CPSR]))+"\n"+
                   "SPSR: "+QString(convertHex(cpu[REG_SPSR_FIQ])));

    cp15TA->setText(QString(CP15HEADER)+
                   "ID(r0): "+QString(convertHex(cp15[CP15_REG0_IDC]))+"\n"+
                   "SCB(r1): "+QString(convertHex(cp15[CP15_REG1_SCB]))+"\n"+
                   "PTE_Hi(r2): "+QString(convertHex(cp15[CP15_REG2_EntryHi]))+"\n"+
                   "PTE_Low(r2): "+QString(convertHex(cp15[CP15_REG2_EntryLo]))+"\n"+
                   "FA(r6): "+QString(convertHex(cp15[CP15_REG6_FA]))+"\n"+
                   "TLBR(r8): "+QString(convertHex(cp15[CP15_REG8_TLBR]))+"\n"+
                   "TLBI(r10): "+QString(convertHex(cp15[CP15_REG10_TLBI]))+"\n"+
                   "CAUSE(r15): "+QString(convertHex(cp15[CP15_REG15_CAUSE])));
}

#endif //QARM_PROCDISPLAY_CC
