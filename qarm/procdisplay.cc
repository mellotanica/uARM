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

#include "procdisplay.h"

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
    cpuReg[10][CPUCOLS-1]->setText(":r9");
    cpuReg[11][CPUCOLS-1]->setText(":r10");
    cpuReg[12][CPUCOLS-1]->setText(":r11");
    cpuReg[13][CPUCOLS-1]->setText(":r12");
    cpuReg[14][CPUCOLS-1]->setText(":r13");
    cpuReg[15][CPUCOLS-1]->setText(":r14");
    cpuReg[18][CPUCOLS-1]->setText(":SPSR");

    cpuReg[0][1]->setText("Usr/Sys");
    cpuReg[0][2]->setText("Svc");
    cpuReg[0][3]->setText("Abt");
    cpuReg[0][4]->setText("Undef");
    cpuReg[0][5]->setText("IRQ");
    cpuReg[0][6]->setText("FIQ");

    cp15Reg[0][0]->setText("CP15 registers:");
    cp15Reg[1][1]->setText("(r0) ID:");
    cp15Reg[2][1]->setText("(r1) SCB:");
    cp15Reg[3][1]->setText("(r1) CCB:");
    cp15Reg[0][3]->setText("(r2) PTE_Hi:");
    cp15Reg[1][3]->setText("(r2) PTE_Low:");
    cp15Reg[2][3]->setText("(r15) Exc:");
    cp15Reg[3][3]->setText("(r15) Int:");
    infoReg[0][0]->setText("BUS Info:");
    infoReg[1][1]->setText("TOD_Hi:");
    infoReg[2][1]->setText("TOD_Low:");
    infoReg[3][1]->setText("TIMER:");

    /*EDIT: coprocessors register set restricted
    for(int i = 1; i < CP15_REGISTERS_NUM+1; i++){
        cp15Reg[i%CP15ROWS][(i/CP15ROWS)*2]->setText("r"+QString::number(i-1)+":");
        cp15Reg[i%CP15ROWS][(i/CP15ROWS)*2]->setAlignment(Qt::AlignRight);
    }*/

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
    cp15Reg[0][4]->setText(convertHex(0));
    cp15Reg[1][4]->setText(convertHex(0));
    cp15Reg[2][4]->setText(convertHex(0));
    cp15Reg[3][4]->setText(convertHex(0));

    infoReg[1][2]->setText(convertHex(0));
    infoReg[2][2]->setText(convertHex(0));
    infoReg[3][2]->setText(convertHex(0));

    /*EDIT: cp15 display fix
    for(int i = 1; i < CP15_REGISTERS_NUM+1; i++)
        cp15Reg[i%CP15ROWS][(i/CP15ROWS)*2+1]->setText(convertHex(0));
        */
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

    cp15Reg[1][2]->setText(convertHex(cp15[0]));
    cp15Reg[2][2]->setText(convertHex(cp15[1]));
    cp15Reg[3][2]->setText(convertHex(cp15[2]));
    cp15Reg[0][4]->setText(convertHex(cp15[3]));
    cp15Reg[1][4]->setText(convertHex(cp15[4]));
    cp15Reg[2][4]->setText(convertHex(cp15[5]));
    cp15Reg[3][4]->setText(convertHex(cp15[6]));

    infoReg[1][2]->setText(convertHex(todH));
    infoReg[2][2]->setText(convertHex(todL));
    infoReg[3][2]->setText(convertHex(timer));

    /*EDIT: ooooold
    for(int i = 1; i < CP15_REGISTERS_NUM+1; i++)
        cp15Reg[i%CP15ROWS][(i/CP15ROWS)*2+1]->setText(convertHex(cp15[i-1]));
        */

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
