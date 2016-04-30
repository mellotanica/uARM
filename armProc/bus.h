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

#include "services/lang.h"
#include "armProc/const.h"
#include "armProc/ramMemory.h"
#include "armProc/event.h"
#include "armProc/blockdev.h"
#include "armProc/mpic.h"
#include "armProc/device.h"
#include "armProc/machine.h"
#include "armProc/processor.h"
#include "armProc/time_stamp.h"
//#include "armProc/mp_controller.h"


//change these values accordignly to linker behavior

#define EXCVTOP      0x00000020
#define DEVBASEADDR  0x00000040
#define DEVTOP       0x000002C0
#define INFOBASEADDR 0x000002D0
#define INFOTOP      0x000002EC
#define BIOSBASEADDR 0x00000300
#define ROMFRAMEBASE 0x00007000
#define ROMFRAMETOP  0x00008000

#define RAMBASEADDR  0x00008000

class Device;
class machine;

class systemBus{
public:
    systemBus(machine *mac);
    ~systemBus();

    void reset();
    void updateRAMTOP();

	Word pipeline[PIPELINE_STAGES];

    bool loadBIOS(char *buffer, Word size);
    bool loadRAM(char *buffer, Word size, bool kernel);

    bool prefetch(Word addr, bool exec);
    bool fetch(Word pc, bool armMode, bool isVirtual);

    ramMemory *getRam() {return ram;}

    AbortType readB(Word *address, Byte *dest);
    AbortType writeB(Word *address, Byte data);
    AbortType readH(Word *address, HalfWord *dest);
    AbortType writeH(Word *address, HalfWord data);
    AbortType readW(Word *address, Word *dest);
    AbortType writeW(Word *address, Word data);

    AbortType writeB(Word *address, Byte data, bool fromProc);
    AbortType writeH(Word *address, HalfWord data, bool fromProc);
    AbortType writeW(Word *address, Word data, bool fromProc);

    // This method increments system clock and decrements interval
    // timer; on timer underflow (0 -> FFFFFFFF transition) a interrupt
    // is generated.  Event queue is checked against the current clock
    // value and device operations are completed if needed; all memory
    // changes are notified to Watch control object
    void ClockTick();

    uint32_t IdleCycles() const;

    void Skip(uint32_t cycles);

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

    // This method returns the current interrupt line status
    Word getPendingInt(const processor* cpu);

    void AssertIRQ(unsigned int il, unsigned int target);
    void DeassertIRQ(unsigned int il, unsigned int target);

    // This method returns the Device object with given "coordinates"
    Device * getDev(unsigned int intL, unsigned int dNum);


    Word getToDLO() const { return TimeStamp::getLo(tod); }
    Word getToDHI() const { return TimeStamp::getHi(tod); }
    Word getTimer() const { return timer; }

    void setToDHI(Word hi);
    void setToDLO(Word lo);
    void setTimer(Word time);

    void SignalTLBChanged(unsigned int index);

    processor* getProcessor(unsigned int cpuId) { return cpus[cpuId]; }

    Word get_unpredictable();
    bool get_unpredictableB();

    void HandleBusAccess(Word pAddr, Word access, processor* cpu);
    void HandleVMAccess(Word asid, Word vaddr, Word access, processor* cpu);

    void signalExecutionEnded();

private:
    machine *mac;
    ramMemory *ram = NULL;
    processor **cpus;
    unsigned int activeCpus = 0;

    uint64_t tod;
    Word timer;

    Word BIOSTOP;
    Word RAMTOP;

    Byte* excVector = NULL;
    Byte* devRegs = NULL;
    Byte* bios = NULL;
    Byte* info = NULL;
    Byte* romFrame = NULL;
    Byte* intBitmap = NULL;

    scoped_ptr<InterruptController> pic;

    //scoped_ptr<MPController> mpController;

    // device events queue
    EventQueue * eventQ = NULL;

    // device handling & interrupt generation tables
    Device* devTable[DEVINTUSED][DEVPERINT];
    Word instDevTable[DEVINTUSED];

    // pending interrupts on lines: this word is packed into MIPS Cause
    // Register IP field format for easy masking
    Word intPendMask;

    // This method accesses the system configuration and constructs
    // the devices needed, linking them to SystemBus object
    Device * makeDev(unsigned int intl, unsigned int dnum);

    AbortType checkAddress(Word *address);

    bool readRomB(Word *address, Byte *dest);
    bool writeRomB(Word *address, Byte data);
    bool readRomH(Word *address, HalfWord *dest);
    bool writeRomH(Word *address, HalfWord data);
    bool readRomW(Word *address, Word *dest);
    bool writeRomW(Word *address, Word data);

    bool initRomW(Word *address, Word data);

    bool getRomVector(Word *address, Byte **romptr);

    void initInfo();
};



#endif //UARM_SYSTEMBUS_H
