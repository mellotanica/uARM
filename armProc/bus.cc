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

#include <string.h>
#include "armProc/bus.h"
#include "armProc/aout.h"
#include "services/utility.h"
#include "services/error.h"
#include "services/debug.h"

// DeviceAreaAddress is a convenience class used to find a specific
// device in bus device area
class DeviceAreaAddress {
public:
    DeviceAreaAddress(Word paddr)
        : pa(paddr)
    {
        assert(DEVBASEADDR <= paddr && paddr < DEVTOP);
    }

    DeviceAreaAddress(unsigned int line, unsigned int device, unsigned int field)
        : pa(DEV_REG_ADDR(line + DEV_IL_START, device) + field * WS)
    {
        assert(line < N_EXT_IL && device < N_DEV_PER_IL && field < DEV_REG_SIZE_W);
    }

    Word address() const { return pa; }

    unsigned int regIndex() const { return wordIndex() / DEV_REG_SIZE_W; }
    unsigned int line() const { return regIndex() / N_DEV_PER_IL; }
    unsigned int device() const { return regIndex() % N_DEV_PER_IL; }
    unsigned int field() const { return wordIndex() % DEV_REG_SIZE_W; }

private:
    unsigned int wordIndex() const { return (pa - DEV_REG_START) >> 2; }

    Word pa;
};

systemBus::systemBus(machine *mac) :
    mac(mac)
{
    if(ram == NULL)
        ram = new ramMemory();
    cpus = new processor*[MachineConfig::MAX_CPUS];

    for (unsigned intl = 0; intl < N_EXT_IL; intl++) {
        for (unsigned int devNo = 0; devNo < N_DEV_PER_IL; devNo++) {
            devTable[intl][devNo] = NULL;
        }
    }

    reset();
}

systemBus::~systemBus(){
    delete eventQ;
    if(ram != NULL){
        delete ram;
        ram = NULL;
    }
    delete [] excVector;
    delete [] devRegs;
    delete [] info;
    delete [] romFrame;
    delete [] intBitmap;
    if(bios != NULL){
        delete [] bios;
        bios = NULL;
    }
    delete [] pipeline;
    for(unsigned int i = 0; i < MachineConfig::MAX_CPUS; i++){
        if(cpus[i] != NULL)
            delete cpus[i];
    }
    delete [] cpus;
}

void systemBus::reset(){
    pic.reset(new InterruptController(this));

    if(bios != NULL)
        delete [] bios;
    bios = NULL;
    tod = UINT64_C(0);
    timer = MAXWORDVAL;
    if(eventQ != NULL)
        delete eventQ;
    eventQ = new EventQueue();

    if(excVector != NULL)
        delete [] excVector;
    excVector = new Byte[EXCVTOP];
    memset(excVector, 0, EXCVTOP);

    if(devRegs != NULL)
        delete [] devRegs;
    devRegs = new Byte[(DEVTOP - DEVBASEADDR)];
    memset(devRegs, 0, (DEVTOP - DEVBASEADDR));

    if(info != NULL)
        delete [] info;
    info = new Byte[(INFOTOP - INFOBASEADDR)];
    memset(info, 0, (INFOTOP - INFOBASEADDR));

    if(romFrame != NULL)
        delete [] romFrame;
    romFrame = new Byte[(ROMFRAMETOP - ROMFRAMEBASE)];
    memset(romFrame, 0, (ROMFRAMETOP - ROMFRAMEBASE));

    if(intBitmap != NULL)
        delete [] intBitmap;
    intBitmap = new Byte[(CDEV_BITMAP_END - CDEV_BITMAP_BASE)];
    memset(intBitmap, 0, (CDEV_BITMAP_END - CDEV_BITMAP_BASE));

    ram->reset(MC_Holder::getInstance()->getConfig()->getRamSize());
    if(activeCpus != MC_Holder::getInstance()->getConfig()->getNumProcessors()){
        //if there will be less cpus then what are active now destroy exceding instances
        //and reset the other ones
        if(activeCpus > MC_Holder::getInstance()->getConfig()->getNumProcessors()){
            for(unsigned int i = 0; i < activeCpus; i++){
                if(i < MC_Holder::getInstance()->getConfig()->getNumProcessors())
                    cpus[i]->reset();
                else
                    delete cpus[i];
            }
        //else reset existing cpus and create the new ones
        } else {
            for(unsigned int i = 0; i < MC_Holder::getInstance()->getConfig()->getNumProcessors(); i++){
                if(i < activeCpus)
                    cpus[i]->reset();
                else
                    cpus[i] = new processor(this);
            }
        }
    } else {
        for(unsigned int i = 0; i < activeCpus; i++)
            cpus[i]->reset();
    }
    // Create devices and initialize registers used for interrupt
    // handling.
    intPendMask = 0UL;
    for (unsigned intl = 0; intl < N_EXT_IL; intl++) {
        instDevTable[intl] = 0UL;
        for (unsigned int devNo = 0; devNo < N_DEV_PER_IL; devNo++) {
            if(devTable[intl][devNo] != NULL)
                delete devTable[intl][devNo];
            devTable[intl][devNo] = makeDev(intl, devNo);
            if (devTable[intl][devNo]->Type() != NULLDEV)
                instDevTable[intl] = SetBit(instDevTable[intl], devNo);
        }
    }
    initInfo();
}

void systemBus::initInfo(){
    Word addr = BUS_REG_RAM_BASE;
    writeW(&addr, RAMBASEADDR);
    addr = BUS_REG_RAM_SIZE;
    RAMTOP = ram->getRamSize();
    writeW(&addr, RAMTOP);
    addr = BUS_REG_DEV_BASE;
    writeW(&addr, DEVBASEADDR);
    tod = 0;
    addr = BUS_REG_TOD_HI;
    writeW(&addr, (Word) ((tod >> 32) & 0xFFFFFFFF));   //TOD Hi
    addr = BUS_REG_TOD_LO;
    writeW(&addr, (Word) (tod & 0xFFFFFFFF));           //TOD Low
    timer = 0xFFFFFFFF;
    addr = BUS_REG_TIMER;
    writeW(&addr, timer);
    addr = BUS_REG_TIME_SCALE;
    writeW(&addr, 1);   //STATIC: timer scale now is fixed to 1 Mz
}

void systemBus::ClockTick(){
    tod++;

    // both registers signal "change" because they are conceptually one
    HandleBusAccess(BUS_REG_TOD_HI, WRITE, NULL);
    HandleBusAccess(BUS_REG_TOD_LO, WRITE, NULL);
    Word addr = BUS_REG_TOD_HI;
    writeW(&addr, (Word) ((tod >> 32) & 0xFFFFFFFF));   //TOD Hi
    addr = BUS_REG_TOD_LO;
    writeW(&addr, (Word) (tod & 0xFFFFFFFF));           //TOD Low

    // Update interval timer
    if (UnsSub(&timer, timer, 1)){
        pic->StartIRQ(IL_TIMER);
    }
    HandleBusAccess(BUS_REG_TIMER, WRITE, NULL);
    addr = BUS_REG_TIMER;
    writeW(&addr, timer);

    // Scan the event queue
    while (!eventQ->IsEmpty() && eventQ->nextDeadline() <= tod) {
        (eventQ->nextCallback())();

        eventQ->RemoveHead();
    }
}

uint32_t systemBus::IdleCycles() const
{
    if (eventQ->IsEmpty())
        return timer;

    const uint64_t et = eventQ->nextDeadline();
    if (et > tod)
        return std::min(timer, (uint32_t) (et - tod - 1));
    else
        return 0;
}

void systemBus::Skip(uint32_t cycles)
{
    tod += cycles;
    HandleBusAccess(BUS_REG_TOD_HI, WRITE, NULL);
    HandleBusAccess(BUS_REG_TOD_LO, WRITE, NULL);
    Word addr = BUS_REG_TOD_HI;
    writeW(&addr, (Word) ((tod >> 32) & 0xFFFFFFFF));   //TOD Hi
    addr = BUS_REG_TOD_LO;
    writeW(&addr, (Word) (tod & 0xFFFFFFFF));           //TOD Low

    timer -= cycles;
    HandleBusAccess(BUS_REG_TIMER, WRITE, NULL);
    addr = BUS_REG_TIMER;
    writeW(&addr, timer);
}

bool systemBus::prefetch(Word addr, bool exec){ //fetches one instruction per execution from exact given address
    //STATIC: should check if accessing VM not bus
    if(exec){ //only check for breakpoints if loading exec stage
        HandleBusAccess(addr, EXEC, NULL);
    }
    pipeline[PIPELINE_EXECUTE] = pipeline[PIPELINE_DECODE];
    pipeline[PIPELINE_DECODE] = pipeline[PIPELINE_FETCH];
    if(readW(&addr, &pipeline[PIPELINE_FETCH]) != ABT_NOABT)
        return false;
    return true;
}

bool systemBus::fetch(Word pc, bool armMode){
    //STATIC: should check if accessing VM not bus
    Word addr = pc - (armMode ? 8 : 4);
    HandleBusAccess(addr, EXEC, NULL);
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
    BIOSTOP = size + BIOSBASEADDR;
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
        Word dataVAddr = 0, textSize = 0;
        bool textSet = false;
        for(Word i = 0; i < size; i++, address++){
            if(textSet && i >= textSize){
                address = dataVAddr;
                textSet = false;
            }
            if(i/4 == (AOUT_HE_DATA_VADDR+1) && i%4 == 0){
                Word taddr = address-4;
                readW(&taddr, &dataVAddr);
            }
            if(!textSet && (i/4 == (AOUT_HE_TEXT_FILESZ+1) && i%4 == 0)){
                Word taddr = address-4;
                readW(&taddr, &textSize);
                textSet = true;
            }
            writeB(&address, (Byte) buffer[i]);
        }
        return true;
    }
    else{   //user program, to be placed somewhere else.. should be loaded via os
        return false;
    }
}

AbortType systemBus::writeB(Word *address, Byte data){
    return writeB(address, data, false);
}

AbortType systemBus::writeH(Word *address, HalfWord data){
    return writeH(address, data, false);
}

AbortType systemBus::writeW(Word *address, Word data){
    return writeW(address, data, false);
}

AbortType systemBus::readB(Word *addr, Byte *dest){
    Word address = *addr;
    AbortType cause = checkAddress(&address);
    HandleBusAccess(address, READ, NULL);
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

AbortType systemBus::writeB(Word *addr, Byte data, bool fromProc){
    Word address = *addr;
    AbortType cause = checkAddress(&address);
    HandleBusAccess(address, WRITE, NULL);
    if(fromProc && address >= BUS_REG_TIMER && address < BUS_REG_TIMER+4){
        pic->EndIRQ(IL_TIMER, 0);   //STATIC: if multiprocessor is implemented this must search for the right cpu
    }
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
    HandleBusAccess(address, READ, NULL);
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

AbortType systemBus::writeH(Word *addr, HalfWord data, bool fromProc){
    Word address = *addr;
    AbortType cause = checkAddress(&address);
    HandleBusAccess(address, WRITE, NULL);
    if(fromProc && address >= BUS_REG_TIMER && address < BUS_REG_TIMER+4){
        pic->EndIRQ(IL_TIMER, 0);   //STATIC: if multiprocessor is implemented this must search for the right cpu
    }
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
    HandleBusAccess(address, READ, NULL);
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

AbortType systemBus::writeW(Word *addr, Word data, bool fromProc){
    Word address = *addr;
    AbortType cause = checkAddress(&address);
    HandleBusAccess(address, WRITE, NULL);
    if(fromProc && address >= BUS_REG_TIMER && address < BUS_REG_TIMER+4){
        pic->EndIRQ(IL_TIMER, 0);   //STATIC: if multiprocessor is implemented this must search for the right cpu
    }
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
    Word addr = *address - (*address % 4);
    if(addr < DEVTOP && addr >= DEVBASEADDR){   //read device register
        DeviceAreaAddress da(addr);
        Device* device = devTable[da.line()][da.device()];
        *dest = (Byte) ((device->ReadDevReg(da.field())) >> ((*address % 4) * 8)) & 0xFF;   
    } else {
        if(!getRomVector(address, &romptr))
            return false;
        *dest = *romptr;
    }
    return true;
}

bool systemBus::writeRomB(Word *address, Byte data){
    Byte *romptr;
    Word addr = *address - (*address % 4);
    if(addr < DEVTOP && addr >= DEVBASEADDR){   //write device register
        DeviceAreaAddress dva(addr);
        Device *device = devTable[dva.line()][dva.device()];
        //addr is now used to edit only the register's byte addressed
        addr = device->ReadDevReg(dva.field());
        addr ^= (0xFF << (*address % 4));
        addr |= (data << (*address % 4));
        device->WriteDevReg(dva.field(), addr);
    } else {
        if(!getRomVector(address, &romptr))
            return false;
        *romptr = data;
    }
    return true;
}

bool systemBus::readRomH(Word *address, HalfWord *dest){
    Byte *romptr;
    Word addr = *address - (*address % 4);
    *dest = 0;
    if(addr < DEVTOP && addr >= DEVBASEADDR){   //read device register
        DeviceAreaAddress da(addr);
        Device* device = devTable[da.line()][da.device()];
        *dest = (HalfWord) ((device->ReadDevReg(da.field())) >> (((*address >> 1) % 2) * 16)) & 0xFFFF;
    } else for(uint i = 0; i < sizeof(HalfWord); i++, addr++){
        if(!getRomVector(&addr, &romptr))
            return false;
        *dest |= (*romptr << (i * 8));
    }
    return true;
}

bool systemBus::writeRomH(Word *address, HalfWord data){
    Byte *romptr;
    Word addr = *address - (*address % 4);
    if(addr < DEVTOP && addr >= DEVBASEADDR){   //write device register
        DeviceAreaAddress dva(addr);
        Device *device = devTable[dva.line()][dva.device()];
        //addr is now used to edit only the register's halfword addressed
        addr = device->ReadDevReg(dva.field());
        addr ^= (0xFFFF << ((*address >> 1) % 2));
        addr |= (data << ((*address >> 1) % 2));
        device->WriteDevReg(dva.field(), addr);
    } else for(uint i = 0; i < sizeof(HalfWord); i++, addr++){
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
    if(addr < DEVTOP && addr >= DEVBASEADDR){   //read device register
        DeviceAreaAddress da(addr);
        Device* device = devTable[da.line()][da.device()];
        *dest = device->ReadDevReg(da.field());
    } else for(uint i = 0; i < sizeof(Word); i++, addr++){
        if(!getRomVector(&addr, &romptr))
            return false;
        *dest |= (*romptr << (i * 8));
    }
    return true;
}

bool systemBus::writeRomW(Word *address, Word data){
    Byte *romptr;
    Word addr = *address;
    if(addr < DEVTOP && addr >= DEVBASEADDR){   //write device register
        DeviceAreaAddress dva(addr);
        Device *device = devTable[dva.line()][dva.device()];
        device->WriteDevReg(dva.field(), data);
    } else for(uint i = 0; i < sizeof(Word); i++, addr++){
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
    else if(*address < BIOSTOP && *address >= BIOSBASEADDR){
        if(bios != NULL)
            *romptr = bios + (offset - BIOSBASEADDR);
        else
            return false;
    }
    else if(*address < INFOTOP && *address >= INFOBASEADDR){
        if(*address < BUS_REG_TIMER+4 && *address >= BUS_REG_TIMER)
            *romptr = (Byte *) &timer + offset - BUS_REG_TIMER;
        else if(*address < BUS_REG_TOD_HI+4 && *address >= BUS_REG_TOD_HI)
            *romptr = (Byte *) &tod + 4 + offset - BUS_REG_TOD_HI;
        else if(*address < BUS_REG_TOD_LO+4 && *address >= BUS_REG_TOD_LO)
            *romptr = (Byte *) &tod + offset - BUS_REG_TOD_LO;
        else
            *romptr = info + (offset - INFOBASEADDR);
    }
    else if(*address < ROMFRAMETOP && *address >= ROMFRAMEBASE)
        *romptr = romFrame + (offset - ROMFRAMEBASE);
    else if(*address < IDEV_BITMAP_END && *address >= IDEV_BITMAP_BASE)
        *romptr = (Byte*) instDevTable + (offset - IDEV_BITMAP_BASE);
    else if(*address < CDEV_BITMAP_END && *address >= CDEV_BITMAP_BASE)
        *romptr = intBitmap + (offset - CDEV_BITMAP_BASE);
    else
        return false;
    return true;
}

void systemBus::IntReq(unsigned int intl, unsigned int devNum){
    pic->StartIRQ(DEV_IL_START + intl, devNum);
}

void systemBus::IntAck(unsigned int intl, unsigned int devNum){
    pic->EndIRQ(DEV_IL_START + intl, devNum);
}

bool systemBus::DMATransfer(Block * blk, Word startAddr, bool toMemory){
    if (BADADDR(startAddr,sizeof(Word)))
        return true;

    AbortType error = ABT_NOABT;

    Word addr;
    if (toMemory) {
        for (Word ofs = 0; ofs < BLOCKSIZE && error == ABT_NOABT; ofs++) {
            addr = startAddr + (ofs * WORDLEN);
            error = writeW(&addr, blk->getWord(ofs));
            HandleBusAccess(startAddr + (ofs * WORDLEN), WRITE, NULL);
        }
    } else {
        Word val;
        for (Word ofs = 0; ofs < BLOCKSIZE && error == ABT_NOABT; ofs++) {
            addr = startAddr + (ofs * WORDLEN);
            error = readW(&addr, &val);
            HandleBusAccess(startAddr + (ofs * WORDLEN), READ, NULL);
            blk->setWord(ofs, val);
        }
    }

    if(error == ABT_NOABT)
        return false;
    return true;
}
bool systemBus::DMAVarTransfer(Block * blk, Word startAddr, Word byteLength, bool toMemory){
    // fit bytes into words
    Word length;
    if (byteLength % WORDLEN)
        length = (byteLength / WORDLEN) + 1;
    else
        length = byteLength / WORDLEN;

    if (BADADDR(startAddr, sizeof(Word)) || length > BLOCKSIZE)
        return true;

    AbortType error = ABT_NOABT;

    Word addr;
    if (toMemory) {
        for (Word ofs = 0; ofs < length && error == ABT_NOABT; ofs++) {
            addr = startAddr + (ofs * WORDLEN);
            error = writeW(&addr, blk->getWord(ofs));
            HandleBusAccess(startAddr + (ofs * WORDLEN), WRITE, NULL);
        }
    } else {
        Word val;
        for (Word ofs = 0; ofs < length && error == ABT_NOABT; ofs++) {
            addr = startAddr + (ofs * WORDLEN);
            error = readW(&addr, &val);
            HandleBusAccess(startAddr + (ofs * WORDLEN), READ, NULL);
            blk->setWord(ofs, val);
        }
    }

    if(error == ABT_NOABT)
        return false;
    return true;
}

Word systemBus::getPendingInt(const processor* cpu){
    return pic->GetIP(cpu->Id());
}

void systemBus::AssertIRQ(unsigned int il, unsigned int target){
    if(cpus != NULL)
        getProcessor(target)->AssertIRQ(il);
}

void systemBus::DeassertIRQ(unsigned int il, unsigned int target){
    if(cpus != NULL)
        getProcessor(target)->DeassertIRQ(il);
}

// This method returns the Device object with given "coordinates"
Device * systemBus::getDev(unsigned int intL, unsigned int dNum)
{
    if (intL < DEVINTUSED  && dNum < DEVPERINT)
        return(devTable[intL][dNum]);
    else {
        Panic("Unknown device specified in SystemBus::getDev()");
        // never returns
        return NULL;
    }
}

// This method accesses the system configuration and constructs
// the devices needed, linking them to SystemBus object
Device* systemBus::makeDev(unsigned int intl, unsigned int dnum)
{
    unsigned int devt;
    Device * dev;

    MachineConfig *config = MC_Holder::getInstance()->getConfig();

    devt = config->getDeviceType(intl, dnum);

    switch(devt) {
    case PRNTDEV:
        dev = new PrinterDevice(this, config, intl, dnum);
        break;

    case TERMDEV:
        dev = new TerminalDevice(this, config, intl, dnum);
        break;

    case ETHDEV:
        dev = new EthDevice(this, config, intl, dnum);
        break;

    case DISKDEV:
        dev = new DiskDevice(this, config, intl, dnum);
        break;

    case TAPEDEV:
        dev = new TapeDevice(this, config, intl, dnum);
        break;

    default:
        dev = new Device(this, intl, dnum);
        break;
    }

    return dev;
}

uint64_t systemBus::scheduleEvent(uint64_t delay, Event::Callback callback){
    return eventQ->InsertQ(tod, delay, callback);
}

void systemBus::setToDHI(Word hi)
{
    TimeStamp::setHi(tod, hi);
}

void systemBus::setToDLO(Word lo)
{
    TimeStamp::setLo(tod, lo);
}

void systemBus::setTimer(Word time)
{
    timer = time;
}

void systemBus::HandleBusAccess(Word pAddr, Word access, processor* cpu){
    mac->HandleBusAccess(pAddr, access, cpu);
}

void systemBus::HandleVMAccess(Word asid, Word vaddr, Word access, processor* cpu){
    mac->HandleVMAccess(asid, vaddr, access, cpu);
}

void systemBus::SignalTLBChanged(unsigned int index){
    mac->updateTLB(index);
}

#endif //UARM_SYSTEMBUS_CC
