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

#ifndef UARM_SYSTEMBUS_H
#define UARM_SYSTEMBUS_H

#include "const.h"
#include "ramMemory.h"
#include "armProc/event.h"
#include "blockdev.h"

//change these values accordignly to linker behavior

#define EXCVTOP      0x00000020
#define DEVBASEADDR  0x00000040
#define DEVTOP       0x000002C0
#define INFOBASEADDR 0x000002D0
#define INFOTOP      0x000002DC
#define BIOSBASEADDR 0x00000300
#define ROMFRAMEBASE 0x00007000
#define ROMFRAMETOP  0x00008000

#define RAMBASEADDR  0x00008000


enum AbortType {
    ABT_NOABT   = 0,
    ABT_MEMERR  = 1,
    ABT_BUSERR  = 2,
    ABT_ADDRERR = 3,
    ABT_SEGERR  = 4,
    ABT_PAGEERR = 5,
    NOABT_ROM   = 0xFF
};

class systemBus{
public:
	systemBus();
    ~systemBus();
	
    void reset();
    void updateRAMTOP();

	Word pipeline[PIPELINE_STAGES];

    bool loadBIOS(char *buffer, Word size);
    bool loadRAM(char *buffer, Word size, bool kernel);

    bool prefetch(Word addr);
    bool fetch(Word pc, bool armMode);
	
    ramMemory *getRam() {return ram;}
	
    AbortType readB(Word *address, Byte *dest);
    AbortType writeB(Word *address, Byte data);
    AbortType readH(Word *address, HalfWord *dest);
    AbortType writeH(Word *address, HalfWord data);
    AbortType readW(Word *address, Word *dest);
    AbortType writeW(Word *address, Word data);

    // This method transfers a block from or to memory, starting with
    // address startAddr; it returns TRUE is transfer was not successful
    // (non-existent memory, read-only memory, unaligned addresses),
    // FALSE otherwise.  It notifies too the memory accesses to Watch
    // control object
    bool DMATransfer(Block * blk, Word startAddr, bool toMemory);

    // This method transfers a partial block from or to memory, starting with
    // address startAddr; it returns TRUE is transfer was not successful
    // (non-existent memory, read-only memory, unaligned addresses),
    // FALSE otherwise.  It notifies too the memory accesses to Watch
    // control object
    bool DMAVarTransfer(Block * blk, Word startAddr, Word byteLength, bool toMemory);

    uint64_t scheduleEvent(uint64_t delay, Event::Callback callback);
    // This method sets the appropriate bits into intCauseDev[] and
    // IntPendMask to signal device interrupt pending; it notifies
    // memory changes to Watch too
    void IntReq(unsigned int intNum, unsigned int devNum);

    // This method resets the appropriate bits into intCauseDev[] and
    // IntPendMask to signal device interrupt acknowlege; it notifies
    // memory changes to Watch too
    void IntAck(unsigned int intNum, unsigned int devNum);

    Word getToDLO() const { return 1; }
    Word getToDHI() const { return 0; }
    Word getTimer() const { return 0; }
    /* good ones
    Word getToDLO() const { return TimeStamp::getLo(tod); }
    Word getToDHI() const { return TimeStamp::getHi(tod); }
    Word getTimer() const { return timer; }
    */

    Word get_unpredictable();
    bool get_unpredictableB();

private:
    ramMemory *ram = NULL;

    Word BIOSTOP;
    Word RAMTOP;

    Byte* excVector;
    Byte* devRegs;
    Byte* bios = NULL;
    Byte* info;
    Byte *romFrame;

    AbortType checkAddress(Word *address);

    bool readRomB(Word *address, Byte *dest);
    bool writeRomB(Word *address, Byte data);
    bool readRomH(Word *address, HalfWord *dest);
    bool writeRomH(Word *address, HalfWord data);
    bool readRomW(Word *address, Word *dest);
    bool writeRomW(Word *address, Word data);

    bool getRomVector(Word *address, Byte **romptr);

    void initInfo();
};



#endif //UARM_SYSTEMBUS_H
