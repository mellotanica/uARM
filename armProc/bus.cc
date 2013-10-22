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

#ifndef UARM_SYSTEMBUS_CC
#define UARM_SYSTEMBUS_CC

#include "bus.h"

systemBus::systemBus(){
    if(ram == NULL)
        ram = new ramMemory();
    excVector = new Byte[EXCVTOP];
    int i;
    for(i = 0; i < EXCVTOP; i++)
        excVector[i] = 0;
    devRegs = new Byte[(DEVTOP - DEVBASEADDR)];
    for(i = 0; i < (DEVTOP - DEVBASEADDR); i++)
        devRegs[i] = 0;
    info = new Byte[(INFOTOP - INFOBASEADDR)];
    bios = NULL;
    initInfo();
}

systemBus::~systemBus(){
    if(ram != NULL){
        delete ram;
        ram = NULL;
    }
}

bool systemBus::prefetch(Word addr){ //fetches one instruction per execution from exact given address
    pipeline[PIPELINE_EXECUTE] = pipeline[PIPELINE_DECODE];
    pipeline[PIPELINE_DECODE] = pipeline[PIPELINE_FETCH];
    if(readW(&addr, &pipeline[PIPELINE_FETCH]) != ABT_NOABT)
        return false;
    return true;
}

bool systemBus::fetch(Word pc, bool armMode){
    Word addr = pc - (armMode ? 8 : 4);
    if(readW(&addr, &pipeline[PIPELINE_EXECUTE]) != ABT_NOABT)
        return false;
    addr += 4;
    if(!armMode){
        if(readW(&addr, &pipeline[PIPELINE_DECODE]) != ABT_NOABT)
            return false;
        pipeline[PIPELINE_FETCH] = 0;
        return true;
    }
    if(readW(&addr, &pipeline[PIPELINE_DECODE]) != ABT_NOABT)
        return false;
    addr += 4;
    if(readW(&addr, &pipeline[PIPELINE_FETCH]) != ABT_NOABT)
        return false;
    return true;
}


Word systemBus::get_unpredictable(){
    Word ret;
    for(unsigned i = 0; i < sizeof(Word) * 8; i++)
        ret |= (rand() % 1 ? 1 : 0) << i;
    return ret;
}

bool systemBus::get_unpredictableB(){
    return rand() % 1;
}

bool systemBus::loadBIOS(char *buffer, Word size){
    BIOSTOP = size + BIOSBASEADDR + 8;
    if(bios != NULL)
        delete [] bios;
    bios = new Byte[size];
    Word address = BIOSBASEADDR;
    for(Word i = 0; i < size; i++, address++){
        writeB(&address, (Byte) buffer[i]);
    }
    return true;
}

bool systemBus::loadRAM(char *buffer, Word size, bool kernel){
    if(kernel){
        Word address = RAMBASEADDR;
        for(Word i = 0; i < size; i++, address++){
           writeB(&address, (Byte) buffer[i]);
        }
        return true;
    }
    else{   //user program, placed somewhere else..

    }
}

AbortType systemBus::readB(Word *addr, Byte *dest){
    Word address = *addr;
    AbortType cause = checkAddress(&address);
    if(cause != ABT_NOABT && cause != NOABT_ROM)
        return cause;
    if(cause == ABT_NOABT){
        if(!ram->read(&address, dest))
            return ABT_MEMERR;
    } else {    //trying to access Bus area
        if(!readRomB(&address, dest))
            return ABT_BUSERR;
    }
    return ABT_NOABT;
}

AbortType systemBus::writeB(Word *addr, Byte data){
    Word address = *addr;
    AbortType cause = checkAddress(&address);
    if(cause != ABT_NOABT && cause != NOABT_ROM)
        return cause;
    if(cause == ABT_NOABT){
        if(!ram->write(&address, data))
            return ABT_MEMERR;
    } else {    //trying to access Bus area
        if(!writeRomB(&address, data))
            return ABT_BUSERR;
    }
    return ABT_NOABT;
}

AbortType systemBus::readH(Word *addr, HalfWord *dest){
    Word address = *addr;
    AbortType cause = checkAddress(&address);
    if(cause != ABT_NOABT && cause != NOABT_ROM)
        return cause;
    if(cause == ABT_NOABT){
        if(!ram->readH(&address, dest))
            return ABT_MEMERR;
    } else {    //trying to access Bus area
        if(!readRomH(&address, dest))
            return ABT_BUSERR;
    }
    return ABT_NOABT;
}

AbortType systemBus::writeH(Word *addr, HalfWord data){
    Word address = *addr;
    AbortType cause = checkAddress(&address);
    if(cause != ABT_NOABT && cause != NOABT_ROM)
        return cause;
    if(cause == ABT_NOABT){
        if(!ram->writeH(&address, data))
            return ABT_MEMERR;
    } else {    //trying to access Bus area
        if(!writeRomH(&address, data))
            return ABT_BUSERR;
    }
    return ABT_NOABT;
}

AbortType systemBus::readW(Word *addr, Word *dest){
    Word address = *addr;
    AbortType cause = checkAddress(&address);
    if(cause != ABT_NOABT && cause != NOABT_ROM)
        return cause;
    if(cause == ABT_NOABT){
        if(!ram->readW(&address, dest))
            return ABT_MEMERR;
    } else {    //trying to access Bus area
        if(!readRomW(&address, dest))
            return ABT_BUSERR;
    }
    return ABT_NOABT;
}

AbortType systemBus::writeW(Word *addr, Word data){
    Word address = *addr;
    AbortType cause = checkAddress(&address);
    if(cause != ABT_NOABT && cause != NOABT_ROM)
        return cause;
    if(cause == ABT_NOABT){
        if(!ram->writeW(&address, data))
            return ABT_MEMERR;
    } else {    //trying to access Bus area
        if(!writeRomW(&address, data))
            return ABT_BUSERR;
    }
    return ABT_NOABT;
}

AbortType systemBus::checkAddress(Word *address){
    //check for address health based on virtual memory state AND cp15 result

    //modify address if needed

    if(*address >= (ram->getRamSize() + RAMBASEADDR))
        return ABT_BUSERR;
    if(*address >= RAMBASEADDR){   //if address points to physical ram, rewrite it to get the right data
        *address -= RAMBASEADDR;
        return ABT_NOABT;
    }
    else
        return NOABT_ROM;
}

bool systemBus::readRomB(Word *address, Byte *dest){
    Byte *romptr;
    if(!getRomVector(address, &romptr))
        return false;
    *dest = *romptr;
    return true;
}

bool systemBus::writeRomB(Word *address, Byte data){
    Byte *romptr;
    if(!getRomVector(address, &romptr))
        return false;
    *romptr = data;
    return true;
}

bool systemBus::readRomH(Word *address, HalfWord *dest){
    Byte *romptr;
    Word addr = *address;
    *dest = 0;
    for(int i = 0; i < sizeof(HalfWord); i++, addr++){
        if(!getRomVector(&addr, &romptr))
            return false;
        *dest |= (*romptr << (i * 8));
    }
    return true;
}

bool systemBus::writeRomH(Word *address, HalfWord data){
    Byte *romptr;
    Word addr = *address;
    for(int i = 0; i < sizeof(HalfWord); i++, addr++){
        if(!getRomVector(&addr, &romptr))
            return false;
        *romptr = (Byte) ((data >> (i * 8)) & 0xFF);
    }
    return true;
}

bool systemBus::readRomW(Word *address, Word *dest){
    Byte *romptr;
    Word addr = *address;
    *dest = 0;
    for(int i = 0; i < sizeof(Word); i++, addr++){
        if(!getRomVector(&addr, &romptr))
            return false;
        *dest |= (*romptr << (i * 8));
    }
    return true;
}

bool systemBus::writeRomW(Word *address, Word data){
    Byte *romptr;
    Word addr = *address;
    for(int i = 0; i < sizeof(Word); i++, addr++){
        if(!getRomVector(&addr, &romptr))
            return false;
        Byte dataB = (Byte) ((data >> (i * 8)) & 0xFF);
        *romptr = dataB;
    }
    return true;
}

bool systemBus::getRomVector(Word *address, Byte **romptr){
    //this will let anyone read and write rom memory, we need to check who is trying to access..

    Word offset = *address + (ENDIANESS_BIGENDIAN ? (3 - 2 * (*address % 4)) : 0);
    if(*address < EXCVTOP)
        *romptr = excVector + offset;
    else if(*address < DEVTOP && *address >= DEVBASEADDR)
        *romptr = devRegs + (offset - DEVBASEADDR);
    else if(*address < BIOSTOP && *address >= BIOSBASEADDR)
        *romptr = bios + (offset - BIOSBASEADDR);
    else if(*address < INFOTOP && *address >= INFOBASEADDR)
        *romptr = info + (offset - INFOBASEADDR);
    else
        return false;
    return true;
}

void systemBus::initInfo(){
    Word addr = INFOBASEADDR;
    writeW(&addr, RAMBASEADDR);
    addr += 4;
    writeW(&addr, RAMTOP);
    addr += 4;
    writeW(&addr, DEVBASEADDR);
}

#endif //UARM_SYSTEMBUS_CC
