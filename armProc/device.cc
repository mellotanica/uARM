/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uMPS - A general purpose computer system simulator
 *
 * Copyright (C) 2004 Mauro Morsiani
 * Copyright (C) 2010 Tomislav Jonjic
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

/****************************************************************************
 *
 * This module contains definitions for Device class and sub-classes.  Each
 * Device sub-class represents a device type inside the simulation; each
 * object is a single device.  Device class iself defines interface for all
 * devices, and represents the "uninstalled" device.  All devices are
 * created and controlled by a SystemBus object.  Each Device is identified
 * by a (interrupt line, device number) pair.  See external documentation
 * for individual devices capabilities and command/error codes.
 *
 ****************************************************************************/

#ifndef UARM_DEVICE_CC
#define UARM_DEVICE_CC

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <boost/bind.hpp>

#include "armProc/const.h"
#include "armProc/types.h"
#include "armProc/blockdev_params.h"

#include "armProc/blockdev.h"
#include "services/util.h"

#include "armProc/device.h"

#include "armProc/machine_config.h"
#include "armProc/time_stamp.h"
#include "services/error.h"
#include "armProc/vde_network.h"
#include "armProc/machine.h"


// last operation result description
HIDDEN const char* const opResult[2] = {
    "UNSUCCESSFUL",
    "SUCCESSFUL"
};


// static buffers definition and costants
#define SMALLBUFSIZE	32
#define STRBUFSIZE 256
HIDDEN char strbuf[STRBUFSIZE];


// common device register definitions
#define STATUS 	0
#define COMMAND 1
#define DATA0   2
#define DATA1   3

// common status conditions
#define READY   1
#define ILOPERR 2
#define BUSY    3

// common command values
#define RESET   0
#define ACK     1

//
// PrinterDevice specific commands, error codes, completion times (in microseconds)
//

// static buffer size
#define PRNTBUFSIZE     128

#define PRNTCHR 2

#define PRNTERR 4

#define PRNTRESETTIME   40
#define PRNTCHRTIME      8

// this means a throughput of 125 KB/s


// TerminalDevice specific definitions, commands, status codes, completion
// times

// static buffer size
#define TERMBUFSIZE     128

// terminal register names
#define RECVSTATUS      0
#define RECVCOMMAND     1
#define TRANSTATUS      2
#define TRANCOMMAND     3

// specific terminal commands
#define TRANCHR 2
#define RECVCHR 2

// specific terminal status conditions
#define TRANERR 4
#define RECVERR 4
#define RECVD   5
#define TRANSMD 5

// terminal op completion times (in microseconds)
#define TERMRESETTIME   400     
#define TRANCHRTIME      80
#define RECVCHRTIME      80

// this means a throughput of about 12.5 KB/s


// DiskDevice specific commands / status codes 

// static buffer size
#define DISKBUFSIZE     128

// controller reset time (microsecs)
#define DISKRESETTIME   400     

// other performance figures are loaded from file

// controller commands
#define SEEKCYL   2
#define READBLK   3
#define WRITEBLK  4

// specific error codes
#define SEEKERR 4
#define READERR 5
#define WRITERR 6
#define DMAERR  7


// TapeDevice specific commands / status codes

// static buffer size
#define TAPEBUFSIZE     128

// specific commands
#define SKIPBLK 2
// READBLK (3) is the same as for DiskDevice
#define BACKBLK 4

// error codes
#define SKIPERR 4
// READERR (5)  and DMAERR (7) as for DiskDevice too
#define BACKERR 6

// tape op completion times (microsecs)
#define TAPERESETTIME   200
#define REWBLKTIME      60
#define BACKBLKTIME     100
#define READBLKTIME     122
#define SKIPBLKTIME     100

// this means a throughput of 2000 KB/s with 1 Mhz DMA transfer, and
// around 4032 KB/s with 99 Mhz DMA transfer
// It would read a full 1GB cartridge in around 9 minutes (1 Mhz DMA)
// It would rewind from end an 1 GB cartridge in around 2 minutes, and
// fast forward it from start to end (or back) in 3 min 50 secs 

// alternative for it would be 100, 200, 360, 200 that would mean (1GB cart.)
// transfer rate of 1024 -> 1440 KB/s in 17 min -> 13 min, full rewind
// in 3 min 50 secs, fast forward in  7 min

// EthDevice specific commands / status codes
// static buffer size
#define ETHBUFSIZE  128

// eth commands
#define READCONF       2
#define READNET        3
#define WRITENET       4
#define CONFIGURE      5

// configuration constants
#define SETMAC 0x80
// already defined in network.e
// #define PROMISQ  0x4
// #define INTERRUPT  0x2
// #define NAMED  0x1


#define READPENDING      0x80
#define READPENDINGMASK  0x7F

// specific error codes
// READERR WRITERR DMAERR as for DiskDevice

// eth op completion times (microsecs)
#define ETHRESETTIME   200
#define READNETTIME    1220
#define WRITENETTIME   READNETTIME
#define CONFNETTIME    40
#define POLLNETTIME    (READNETTIME / 2)

//
// local functions
//

// This function decodes device STATUS field and tells if previous operation
// has been successful or not
HIDDEN const char * isSuccess(unsigned int devType, Word regVal);


/****************************************************************************/
/* Definitions to be exported.                                              */
/****************************************************************************/

// Device class defines the interface to all device types, and represents
// the "uninstalled device" (NULLDEV) itself. Device objects are created and
// controlled by a SystemBus object, but also may be inspected by Watch if
// needed

// This method creates a Device object with "coordinates" (interrupt line,
// device number) clears device register, and links it to bus
Device::Device(systemBus * busl, unsigned int intl, unsigned int dnum)
{
    intL = intl;
    devNum = dnum;
    dType = NULLDEV;
    for (unsigned int i = 0; i < DEVREGLEN; i++)
        reg[i] = 0UL;
    bus = busl;
    complTime = UINT64_C(0);
    // a NULLDEV never works
    isWorking = false;
}

// No operation for "uninstalled" devices
Device::~Device()
{}

// This method is invoked by SystemBus when the required operation scheduled
// on the device (using Event objects and queue) should be completed: the
// default NULLDEV device has nothing to do, but others do
unsigned int Device::CompleteDevOp()
{
    return 0;
}

// This method allows SystemBus to write into device register for device:
// NULLDEV device register write has no effects, but other devices will
// start performing required operations if COMMAND register is written with
// proper codes
void Device::WriteDevReg(unsigned int dummynum, Word dummydata)
{
}

// This method returns the static buffer contained in each device describing
// the current device status (operation performed, etc.). NULLDEV devices
// are not operational
const char* Device::getDevSStr()
{
    return "Not operational";
}

// This method returns the current value for device register field indexed
// by regnum
Word Device::ReadDevReg(unsigned int regnum)
{
    if (regnum < DEVREGLEN) {
        return(reg[regnum]);
    } else {
        Panic("Unknown register index in Device::ReadDevReg()");
        // never returns        
        return MAXWORDVAL;
    }
}

// This method gets the current operational status for the device, as set by
// user inside the simulation; a "not-working" device fails all operations
// requested and reports proper error codes; a NULLDEV always fail
bool Device::getDevNotWorking()
{
    return !isWorking;
}

/*
 * Return a human-readable expression for completion time of the
 * current device operation.
 */
std::string Device::getCTimeInfo() const
{
    if (isBusy())
        return TimeStamp::toString(complTime);
    else
        return "";
}

// This method sets the operational status for the device inside the
// simulation as user wishes; a "not-working" device fails all operations
// requested and reports proper error codes; a NULLDEV always fails.
bool Device::setDevNotWorking(bool cond)
{
    if (dType == NULLDEV)
        isWorking = false;
    else
        isWorking = !cond;

    return !isWorking;
}

void Device::setCondition(bool working)
{
    if (dType != NULLDEV && working != isWorking) {
        isWorking = working;
        emit SignalConditionChanged(isWorking);
    }
}

// This method allows to copy inputstr contents inside TerminalDevice
// receiver buffer: not operational for all other devices (NULLDEV included)
// and produces a panic message
void Device::Input(const char * inputstr)
{
    Panic("Input directed to a non-Terminal device in Device::Input()");
}

// This method allows to load/unload tapes inside a TapeDevice. For it, if
// tFName == NULL or EMPTYSTR, method returns TRUE if a new tape may be
// loaded, FALSE otherwise; else, if tFName != NULL it tries to load the
// specified tape file, returning completion status. This method is not
// operational for other devices (NULLDEV included) and produces a panic
// message
bool Device::TapeLoad(const char * tFName)
{
    Panic("Tape load request to a non-Tape device in Device::TapeLoad()");
    // no real return
    return(false);
}

bool Device::isBusy() const
{
    return reg[STATUS] == BUSY;
}

uint64_t Device::scheduleIOEvent(uint64_t delay)
{
    return bus->scheduleEvent(delay, boost::bind(&Device::CompleteDevOp, this));
}

/****************************************************************************/

// PrinterDevice class allows to emulate parallel character printer
// currently in use (see performance figures shown before). It uses the same
// interface as Device, redefining only a few methods' implementation: refer
// to it for individual methods descriptions. 
// It adds to Device data structure:
// a pointer to SetupInfo object containing printer log file name;
// a static buffer for device operation & status description;
// a FILE structure for log file access.

//
// See Device class methods description for interface
//

PrinterDevice::PrinterDevice(systemBus* bus, const MachineConfig* cfg,
                             unsigned int il, unsigned int devNo)
    : Device(bus, il, devNo)
    , config(cfg)
{
    dType = PRNTDEV;
    isWorking = true;
    reg[STATUS] = READY;
    sprintf(statStr, "Idle");

    if ((prntFile = fopen(config->getDeviceFile(il, devNo).c_str(), "w")) == NULL) {
        sprintf(strbuf, "Cannot open printer %u file : %s", devNum, strerror(errno));
        Panic(strbuf);
    }
}

PrinterDevice::~PrinterDevice()
{
    // tries to close log file
    if (fclose(prntFile) == EOF) {
        sprintf(strbuf, "Cannot close printer file %u : %s", devNum, strerror(errno));
        Panic(strbuf);
    }
}

void PrinterDevice::WriteDevReg(unsigned int regnum, Word data)
{
    // Only COMMAND and DATA0 registers are writable, and only when 
    // device is not busy.
    if (reg[STATUS] == BUSY)
        return;

    switch (regnum) {
    case COMMAND:
        reg[COMMAND] = data;

        // decode operation requested: for each, acknowledges a
        // previous interrupt if pending, sets the device registers,
        // and inserts an Event in SystemBus mantained queue
        switch (data) {
        case RESET:
            bus->IntAck(intL, devNum);
            complTime = scheduleIOEvent(PRNTRESETTIME * config->getClockRate());
            sprintf(statStr, "Resetting (last op: %s)", isSuccess(dType, reg[STATUS]));
            reg[STATUS] = BUSY;
            break;

        case ACK:
            bus->IntAck(intL, devNum);
            sprintf(statStr, "Idle (last op: %s)", isSuccess(dType, reg[STATUS]));
            reg[STATUS] = READY;
            break;

        case PRNTCHR:
            bus->IntAck(intL, devNum);
            sprintf(statStr, "Printing char 0x%.2X (last op: %s)", 
                    (unsigned char) reg[DATA0], isSuccess(dType, reg[STATUS]));
            complTime = scheduleIOEvent(PRNTCHRTIME * config->getClockRate());
            reg[STATUS] = BUSY;
            break;

        default:
            sprintf(statStr, "Unknown command (last op: %s)", isSuccess(dType, reg[STATUS]));
            reg[STATUS] = ILOPERR;
            bus->IntReq(intL, devNum);
            break;
        }

        // Status has changed (almost certanly, that is -- we don't
        // worry about spurious status change notifications as they
        // are harmless).
        emit SignalStatusChanged(getDevSStr());
        break;

    case DATA0:
        reg[DATA0] = data;
        break;

    default:
        break;
    }
}

const char* PrinterDevice::getDevSStr()
{
    return statStr;
}

unsigned int PrinterDevice::CompleteDevOp()
{
    // checks which operation must be completed: for each, sets device
    // register, performs requested operation and produces an interrupt
    // request

    switch (reg[COMMAND]) {
    case RESET:
        // a reset always works, even if isWorking == FALSE
        sprintf(statStr, "Reset completed : waiting for ACK");
        reg[STATUS] = READY;
        break;

    case PRNTCHR:
        if (isWorking) {
            // normal operation
            if (fputc((unsigned char) reg[DATA0], prntFile) == EOF) {
                sprintf(strbuf, "Error writing printer %u file : %s", devNum, strerror(errno));
                Panic(strbuf);
            }
            fflush(prntFile);
            sprintf(statStr, "Printed char 0x%.2X : waiting for ACK", (unsigned char) reg[DATA0]);
            reg[STATUS] = READY;
        } else {
            // no operation & error simulation
            sprintf(statStr, "Error printing char 0x%.2X : waiting for ACK", (unsigned char) reg[DATA0]);
            reg[STATUS] = PRNTERR;
        }
        break;

    default:
        Panic("Unknown operation in PrinterDevice::CompleteDevOp()");
        break;
    }

    emit SignalStatusChanged(getDevSStr());

    bus->IntReq(intL, devNum);

    return STATUS;
}


// TerminalDevice class allows to emulate serial "dumb" terminal (see
// performance figures shown before). TerminalDevice may be split up into
// two independent sub-devices: "transmitter" and "receiver".
// It uses the same interface as Device, redefining only a few methods'
// implementation: refer to it for individual methods descriptions.
// It adds to Device data structure:
// a pointer to SetupInfo object containing terminal log file name;
// a static buffer for device operation & status description;
// a FILE structure for log file access;
// some structures for handling terminal transmitter and receiver.

//
// See Device class methods description for interface
//

TerminalDevice::TerminalDevice(systemBus* bus, const MachineConfig* cfg,
                               unsigned int il, unsigned int devNo)
    : Device(bus, il, devNo),
      config(cfg)
{
    dType = TERMDEV;
    isWorking = true;
    recvBuf = NULL;
    recvBp = 0;
    reg[RECVSTATUS] = READY;
    reg[TRANSTATUS] = READY;
    sprintf(recvStatStr, "Idle");
    sprintf(tranStatStr, "Idle");
    recvCTime = UINT64_C(0);
    tranCTime = UINT64_C(0);
    recvIntPend = false;
    tranIntPend = false;

    // tries to open log file
    if ((termFile = fopen(config->getDeviceFile(il, devNo).c_str(), "w")) == NULL) {
        sprintf(strbuf, "Cannot open terminal %u file : %s", devNum, strerror(errno));
        Panic(strbuf);
    }
    // else file has been open with success: set it to no buffering for quick 
    // terminal screen update
    setvbuf(termFile, (char *) NULL, _IONBF, 0);
}

TerminalDevice::~TerminalDevice()
{
    if (fclose(termFile) == EOF) {
        sprintf(strbuf, "Cannot close terminal file %u : %s", devNum, strerror(errno));
        Panic(strbuf);
    }
}

void TerminalDevice::WriteDevReg(unsigned int regnum, Word data)
{
    // only COMMAND registers are writable, and only when device is not busy
    // format is NNNN NNNN CHAR COMM

    switch (regnum) {
    case RECVCOMMAND:
        // decode operation requested: for each, acknowledges a
        // previous interrupt if pending, sets the device registers,
        // and inserts an Event in SystemBus mantained queue
        if (reg[RECVSTATUS] != BUSY) {
            reg[RECVCOMMAND] = data;

            switch (data) {
            case RESET:
                if (!tranIntPend)
                    bus->IntAck(intL, devNum);
                recvIntPend = false;
                recvCTime = scheduleIOEvent(TERMRESETTIME * config->getClockRate());
                sprintf(recvStatStr, "Resetting (last op: %s)",
                        isSuccess(dType, reg[RECVSTATUS] & BYTEMASK));
                reg[RECVSTATUS] = BUSY;
                break;

            case ACK:
                if (!tranIntPend)
                    bus->IntAck(intL, devNum);
                recvIntPend = false;
                sprintf(recvStatStr, "Idle (last op: %s)",
                        isSuccess(dType, reg[RECVSTATUS] & BYTEMASK));
                reg[RECVSTATUS] = READY;
                break;

            case RECVCHR:
                if (!tranIntPend)
                    bus->IntAck(intL, devNum);
                recvIntPend = false;
                sprintf(recvStatStr, "Receiving (last op: %s)",
                        isSuccess(dType, reg[RECVSTATUS] & BYTEMASK));
                recvCTime = scheduleIOEvent(RECVCHRTIME * config->getClockRate());
                reg[RECVSTATUS] = BUSY;
                break;

            default:
                sprintf(recvStatStr, "Unknown command (last op: %s)",
                        isSuccess(dType, reg[RECVSTATUS] & BYTEMASK));
                reg[RECVSTATUS] = ILOPERR;
                bus->IntReq(intL, devNum);
                recvIntPend = true;
                break;
            }
            emit SignalStatusChanged(getDevSStr());
        }
        break;

    case TRANCOMMAND:
        // decode operation requested: for each, acknowledges a
        // previous interrupt if pending, sets the device registers,
        // and inserts an Event in SystemBus mantained queue
        if (reg[TRANSTATUS] != BUSY) {
            reg[TRANCOMMAND] = data;

            // to extract command
            switch (data & BYTEMASK) {
            case RESET:
                if (!recvIntPend)
                    bus->IntAck(intL, devNum);
                tranIntPend = false;
                tranCTime = scheduleIOEvent(TERMRESETTIME * config->getClockRate());
                sprintf(tranStatStr, "Resetting (last op: %s)",
                        isSuccess(dType, reg[TRANSTATUS] & BYTEMASK));
                reg[TRANSTATUS] = BUSY;
                break;

            case ACK:
                if (!recvIntPend)
                    bus->IntAck(intL, devNum);
                tranIntPend = false;
                sprintf(tranStatStr, "Idle (last op: %s)",
                        isSuccess(dType, reg[TRANSTATUS] & BYTEMASK));
                reg[TRANSTATUS] = READY;
                break;

            case TRANCHR:
                if (!recvIntPend)
                    bus->IntAck(intL, devNum);
                tranIntPend = false;
                sprintf(tranStatStr, "Transm. char 0x%.2X (last op: %s)",
                        (unsigned char) ((data >> BYTELEN) & BYTEMASK),
                        isSuccess(dType, reg[TRANSTATUS] & BYTEMASK));

                tranCTime = scheduleIOEvent(TRANCHRTIME * config->getClockRate());
                reg[TRANSTATUS] = BUSY;
                break;

            default:
                sprintf(tranStatStr, "Unknown command (last op: %s)",
                        isSuccess(dType, reg[TRANSTATUS] & BYTEMASK));
                reg[TRANSTATUS] = ILOPERR;
                bus->IntReq(intL, devNum);
                tranIntPend = true;
                break;
            }
            emit SignalStatusChanged(getDevSStr());
        }
        break;

    case RECVSTATUS:
    case TRANSTATUS:
    default:
        break;
    }
}

const char* TerminalDevice::getDevSStr()
{
    sprintf(strbuf, "%s\n%s", recvStatStr, tranStatStr); 
    return strbuf;
}

const char* TerminalDevice::getTXStatus() const
{
    return tranStatStr;
}

const char* TerminalDevice::getRXStatus() const
{
    return recvStatStr;
}

std::string TerminalDevice::getCTimeInfo() const
{
    return getRXCTimeInfo() + "\n" + getTXCTimeInfo();
}

std::string TerminalDevice::getTXCTimeInfo() const
{
    if (reg[TRANSTATUS] == BUSY)
        return TimeStamp::toString(tranCTime);
    else
        return "";
}

std::string TerminalDevice::getRXCTimeInfo() const
{
    if (reg[RECVSTATUS] == BUSY)
        return TimeStamp::toString(recvCTime);
    else
        return "";
}

unsigned int TerminalDevice::CompleteDevOp()
{
    // only one sub-device should complete its op: which one?
    bool doRecv;
    unsigned int devMod;

    // determines which operation must be completed
    if (reg[RECVSTATUS] == BUSY && reg[TRANSTATUS] == BUSY) {
        // Both sub-devices are working, so tie breaking depends on
        // timestamps: lower is first. If they are equal, this doesn't
        // matter because there will be another CompleteDevOp
        // following, and one sub-device will have already completed
        // its op or posponed it (recv).
        if (recvCTime <= tranCTime)
            doRecv = true;
        else
            doRecv = false;
    } else {
        // Surely one or other sub-device is idle
        doRecv = (reg[RECVSTATUS] == BUSY);
    }

    if (doRecv) {
        // recv sub-device operation completion
        //
        // Checks which operation must be completed: for each, sets
        // device register, performs requested operation and produces
        // an interrupt request.
        switch (reg[RECVCOMMAND]) {
        case RESET:
            // a reset always works, even if isWorking == FALSE
            sprintf(recvStatStr, "Reset completed : waiting for ACK");
            reg[RECVSTATUS] = READY;
            recvIntPend = true;
            bus->IntReq(intL, devNum);
            break;

        case RECVCHR:
            if (recvBuf == NULL || recvBuf[recvBp] == EOS) {
                // no char in input: wait another receiver cycle
                recvCTime = scheduleIOEvent(RECVCHRTIME * config->getClockRate());
            } else {
                // buffer is not empty
                if (isWorking) {
                    sprintf(recvStatStr, "Received char 0x%.2X : waiting for ACK", recvBuf[recvBp]);
                    reg[RECVSTATUS] = (((Word) recvBuf[recvBp]) << BYTELEN) | RECVD;
                    recvBp++;
                } else {
                    // no operation & error simulation
                    sprintf(recvStatStr, "Error receiving char : waiting for ACK");
                    reg[RECVSTATUS] = RECVERR;
                }
                // interrupt request
                recvIntPend = true;
                bus->IntReq(intL, devNum);
            }
            break;

        default:
            Panic("Unknown operation in TerminalDevice::CompleteDevOp()");
            break;
        }
        // even if there is no char to receive, the RECVSTATUS register is
        // "rewritten" by the receiver and CTime changed
        devMod = RECVSTATUS;
    } else {
        // Transmitter sub-device operation completion:
        //
        // Checks which operation must be completed: for each, sets
        // device register, performs requested operation and produces
        // an interrupt request.
        switch (reg[TRANCOMMAND] & BYTEMASK) {
        case RESET:
            // a reset always works, even if isWorking == FALSE
            sprintf(tranStatStr, "Reset completed : waiting for ACK");
            reg[TRANSTATUS] = READY;
            break;

        case TRANCHR:
            if (isWorking) {
                if (fputc((unsigned char) ((reg[TRANCOMMAND] >> BYTELEN) & BYTEMASK), termFile) == EOF) {
                    sprintf(strbuf, "Error writing terminal %u file : %s", devNum, strerror(errno));
                    Panic(strbuf);
                }
                // else operation is successful:
                fflush(termFile);
                emit SignalTransmitted((unsigned char) ((reg[TRANCOMMAND] >> BYTELEN) & BYTEMASK));
                sprintf(tranStatStr, "Transm. char 0x%.2lX : waiting for ACK",
                        (reg[TRANCOMMAND] >> BYTELEN) & BYTEMASK);
                reg[TRANSTATUS] = (reg[TRANCOMMAND] & (BYTEMASK << BYTELEN)) | TRANSMD;
            } else {
                // no operation & error simulation
                sprintf(tranStatStr, "Error transm. char 0x%.2lX : waiting for ACK",
                        (reg[TRANCOMMAND] >> BYTELEN) & BYTEMASK);
                reg[TRANSTATUS] = (reg[TRANCOMMAND] & (BYTEMASK << BYTELEN)) | TRANERR;
            }
            break;

        default:
            Panic("Unknown operation in TerminalDevice::CompleteDevOp()");
            break;
        }
        // interrupt generation 
        bus->IntReq(intL, devNum);
        tranIntPend = true;
        devMod = TRANSTATUS;
    }
    emit SignalStatusChanged(getDevSStr());

    bus->HandleBusAccess(DEV_REG_ADDR(intL, devNum) + devMod * WS, WRITE, NULL);

    return devMod;
}

void TerminalDevice::Input(const char* inputstr)
{
    char* strp;

    if (recvBuf != NULL && recvBuf[recvBp] == EOS) {
        // buffer exausted: delete it
        delete recvBuf;
        recvBuf = NULL;
    }

    if (recvBuf == NULL) {
        // simply copies inputstr into it, adding a trailing '\n'
        recvBuf = new char[strlen(inputstr) + 2];
        sprintf(recvBuf, "%s\n", inputstr);
    } else {
        // copies the unreceived buffer part plus the new input into
        // a new buffer
        strp = new char [(strlen(recvBuf) - recvBp) + strlen(inputstr) + 2];
        sprintf(strp, "%s%s\n",&(recvBuf[recvBp]), inputstr);
        delete recvBuf;
        recvBuf = strp;
    }
    recvBp = 0;

    // writes input to log file 
    if (fprintf(termFile, "%s\n", inputstr) < 0) {
        sprintf(strbuf, "Error writing terminal %u file : %s", devNum, strerror(errno));
        Panic(strbuf);
    }
}


// DiskDevice class allows to emulate a disk drive: each 512 byte sector it
// contains is identified by (cyl, head, sect) set of disk coordinates;
// (geometry and performance figures are loaded from disk image file). 
// Operations on sectors (R/W) require previous seek on the desired cylinder.
// It also contains a sector buffer of one sector to speed up operations.
//
// It uses the same interface as Device, redefining only a few methods'
// implementation: refer to it for individual methods descriptions.
//
// It adds to Device data structure:
// a pointer to SetupInfo object containing disk image file name;
// a static buffer for device operation & status description;
// a FILE structure for disk image file access;
// a set of disk parameters (read from disk image file header);
// a Block object for file handling;
// some items for performance computation.

DiskDevice::DiskDevice(systemBus* bus, const MachineConfig* cfg,
                       unsigned int line, unsigned int devNo)
    : Device(bus, line, devNo)
    , config(cfg)
{
    // adds to a Device object DiskDevice-specific fields
    dType = DISKDEV;
    isWorking = true;
    reg[STATUS] = READY;
    sprintf(statStr, "Idle");
    diskBuf = new Block();

    // tries to access disk image file 
    if ((diskFile = fopen(config->getDeviceFile(intL, devNum).c_str(), "r+")) == NULL) {
        sprintf(strbuf, "Cannot open disk %u file : %s", devNum, strerror(errno));
        Panic(strbuf);
    }

    // else file has been open with success: tests if it is a valid disk file
    diskP = new DriveParams(diskFile, &diskOfs);

    if (diskOfs == 0) {
        // file is not a valid disk file
        sprintf(strbuf, "Cannot open disk %u file : invalid/corrupted file", devNum);
        Panic(strbuf);
    }

    // DATA1 format == drive geometry: CYL CYL HEAD SECT
    reg[DATA1] = (diskP->getCylNum() << HWORDLEN) | (diskP->getHeadNum() << BYTELEN) | diskP->getSectNum();

    currCyl = 0;
    sectTicks = (diskP->getRotTime() * config->getClockRate()) / diskP->getSectNum();
    cylBuf = headBuf = sectBuf = MAXWORDVAL;
}

DiskDevice::~DiskDevice()
{
    delete diskBuf;
    delete diskP;

    if (fclose(diskFile) == EOF) {
        sprintf(strbuf, "Cannot close disk file %u : %s", devNum, strerror(errno));
        Panic(strbuf);
    }
}

// Disk device register write: only COMMAND and DATA0 registers are
// writable, and only when device is not busy.

void DiskDevice::WriteDevReg(unsigned int regnum, Word data)
{
    if (reg[STATUS] == BUSY)
        return;

    Word timeOfs;
    unsigned int cyl, head, sect, currSect;

    switch (regnum) {
    case COMMAND:
        reg[COMMAND] = data;

        // Decode operation requested: for each, acknowledges a
        // previous interrupt if pending, sets the device registers,
        // and inserts an Event in SystemBus mantained queue.
        switch (data & BYTEMASK) {
        case RESET:
            bus->IntAck(intL, devNum);
            // controller reset & cylinder recalibration
            timeOfs = (DISKRESETTIME + (diskP->getSeekTime() * currCyl)) * config->getClockRate();
            complTime = scheduleIOEvent(timeOfs);
            sprintf(statStr, "Resetting (last op: %s)", isSuccess(dType, reg[STATUS]));
            reg[STATUS] = BUSY;
            break;

        case ACK:
            bus->IntAck(intL, devNum);
            sprintf(statStr, "Idle (last op: %s)", isSuccess(dType, reg[STATUS]));
            reg[STATUS] = READY;
            break;

        case SEEKCYL:
            bus->IntAck(intL, devNum);
            cyl = (data >> BYTELEN) & IMMMASK;
            if (cyl < diskP->getCylNum()) {
                bus->IntAck(intL, devNum);
                sprintf(statStr, "Seeking Cyl 0x%.4X (last op: %s)", cyl, isSuccess(dType, reg[STATUS]));
                // compute movement offset 
                if (cyl < currCyl)
                    cyl = currCyl - cyl;
                else
                    cyl = cyl - currCyl;
                complTime = scheduleIOEvent((diskP->getSeekTime() * cyl * config->getClockRate()) + 1);
                reg[STATUS] = BUSY;
            } else {
                // cyl out of range
                sprintf(statStr, "Cyl 0x%.4X out of range : waiting for ACK", cyl);
                reg[STATUS] = SEEKERR;
                bus->IntReq(intL, devNum);
            }
            break;

        case READBLK:
            bus->IntAck(intL, devNum);
            // computes target coordinates
            head = (data >> HWORDLEN) & BYTEMASK;
            sect = (data >> BYTELEN) & BYTEMASK;
            if (head < diskP->getHeadNum() && sect < diskP->getSectNum()) {
                sprintf(statStr, "Reading C/H/S 0x%.4X/0x%.2X/0x%.2X (last op: %s)",
                        currCyl, head, sect, isSuccess(dType, reg[STATUS]));
                if (currCyl == cylBuf && head == headBuf && sect == sectBuf) {
                    // sector is already in disk buffer
                    timeOfs = DMATICKS;
                } else {
                    // invalidate current buffer
                    cylBuf = headBuf = sectBuf = MAXWORDVAL;

                    // compute op completion time

                    // use only TodLO for easier computation
                    currSect = (bus->getToDLO() / sectTicks) % diskP->getSectNum();

                    // remaining time for current sector
                    timeOfs = bus->getToDLO() % sectTicks;

                    // compute sector offset
                    if (sect > currSect)
                        sect = (sect - currSect) - 1;
                    else
                        sect = (diskP->getSectNum() - 1) - (currSect - sect); 

                    // completion time is = current sect rem. time + 
                    //   sectors-in-between time + sector data read +
                    // DMA transfer time
                    timeOfs += (sectTicks * sect) + ((sectTicks * diskP->getDataSect()) / 100) + DMATICKS; 
                }
                complTime = scheduleIOEvent(timeOfs);
                reg[STATUS] = BUSY;
            } else {
                // head/sector out of range 
                sprintf(statStr, "Head/sect 0x%.2X/0x%.2X out of range : waiting for ACK", head, sect);
                reg[STATUS] = READERR;
                bus->IntReq(intL, devNum);
            }   
            break;

        case WRITEBLK:
            bus->IntAck(intL, devNum);
            // computes target coordinates
            head = (data >> HWORDLEN) & BYTEMASK;
            sect = (data >> BYTELEN) & BYTEMASK;
            if (head < diskP->getHeadNum() && sect < diskP->getSectNum()) {
                sprintf(statStr, "Writing C/H/S 0x%.4X/0x%.2X/0x%.2X (last op: %s)",
                        currCyl, head, sect, isSuccess(dType, reg[STATUS]));
                // DMA transfer from memory
                if (bus->DMATransfer(diskBuf, reg[DATA0], false)) {
                    // DMA transfer error: invalidate current buffer
                    cylBuf = headBuf = sectBuf = MAXWORDVAL;
                    timeOfs = DMATICKS;
                } else {
                    // disk sector in buffer from memory
                    cylBuf = currCyl;
                    headBuf = head;
                    sectBuf = sect;     
                                                                        
                    // compute op completion time
                                                                        
                    // use only TodLO for easier computation
                    // disk spins during DMA transfer
                    currSect = ((bus->getToDLO() + DMATICKS) / sectTicks) % diskP->getSectNum();
                                                                
                    // remaining time for DMA + current sector
                    timeOfs = DMATICKS + ((bus->getToDLO() + DMATICKS) % sectTicks);
                                                                
                    // compute sector offset
                    if (sect > currSect)
                        sect = (sect - currSect) - 1;
                    else
                        sect = (diskP->getSectNum() - 1) - (currSect - sect); 
                                                        
                    // completion time is = DMA time + current sect rem. time + 
                    //   sectors-in-between time + sector data write
                    timeOfs += (sectTicks * sect) + ((sectTicks * diskP->getDataSect()) / 100);
                }
                complTime = scheduleIOEvent(timeOfs);
                reg[STATUS] = BUSY;
            } else {
                // head/sector out of range 
                sprintf(statStr, "Head/sect 0x%.2X/0x%.2X out of range : waiting for ACK", head, sect);
                reg[STATUS] = WRITERR;
                bus->IntReq(intL, devNum);
            }
            break;

        default:
            sprintf(statStr, "Unknown command (last op: %s)", isSuccess(dType, reg[STATUS]));
            reg[STATUS] = ILOPERR;
            bus->IntReq(intL, devNum);
            break;
        }

        emit SignalStatusChanged(getDevSStr());
        break;

    case DATA0:
        // physical address for R/W buffer in memory
        reg[DATA0] = data;
        break;

    default:
        break;
    }
}

const char* DiskDevice::getDevSStr()
{
    return statStr;
}

unsigned int DiskDevice::CompleteDevOp()
{
    // for file access
    SWord blkOfs;
    unsigned int head, sect;

    // checks which operation must be completed: for each, sets device
    // register, performs requested operation and produces an interrupt
    // request
    switch (reg[COMMAND] & BYTEMASK) {
    case RESET:
        // a reset always works, even if isWorking == FALSE
        // it invalidates the sector buffer
        sprintf(statStr, "Reset completed : waiting for ACK");
        reg[STATUS] = READY;
        cylBuf = headBuf = sectBuf = MAXWORDVAL;
        break;

    case SEEKCYL:
        if (isWorking) {
            currCyl = (reg[COMMAND] >> BYTELEN) & IMMMASK;
            sprintf(statStr, "Cyl 0x%.4X reached : waiting for ACK", currCyl);
            reg[STATUS] = READY;
        } else {
            // error simulation: currCyl is between seek start & end  
            currCyl = (((reg[COMMAND] >> BYTELEN) & IMMMASK) + currCyl) / 2;
            sprintf(statStr, "Cyl 0x%.4X seek error : waiting for ACK", currCyl);
            reg[STATUS] = SEEKERR;
        }
        break;

    case READBLK:
        // locates target coordinates
        head = (reg[COMMAND] >> HWORDLEN) & BYTEMASK;
        sect = (reg[COMMAND] >> BYTELEN) & BYTEMASK;
        if (isWorking) {
            blkOfs = (diskOfs + ((currCyl * diskP->getHeadNum() * diskP->getSectNum()) +
                                 (head * diskP->getSectNum()) + sect) * BLOCKSIZE) * WORDLEN;

            if (cylBuf != MAXWORDVAL || !diskBuf->ReadBlock(diskFile, blkOfs)) {
                // Wanted sector is already in buffer or has been read correctly
                cylBuf = currCyl;
                headBuf = head;
                sectBuf = sect;
                if (bus->DMATransfer(diskBuf, reg[DATA0], true)) {
                    // DMA transfer error
                    reg[STATUS] = DMAERR;
                    sprintf(statStr, "DMA error reading C/H/S 0x%.4X/0x%.2X/0x%.2X : waiting for ACK",
                            currCyl, head, sect);
                } else {
                    // all ok
                    sprintf(statStr, "C/H/S 0x%.4X/0x%.2X/0x%.2X block read: waiting for ACK",
                            currCyl, head, sect);
                    reg[STATUS] = READY;
                }
            } else {
                // ReadBlock() has failed for sure
                sprintf(strbuf, "Unable to read disk %u file : invalid/corrupted file", devNum);
                Panic(strbuf);
            }
        } else {
            // error simulation
            sprintf(statStr, "Error reading C/H/S 0x%.4X/0x%.2X/0x%.2X : waiting for ACK",
                    currCyl, head, sect);
            // buffer invalidation
            cylBuf = headBuf = sectBuf = MAXWORDVAL;
            reg[STATUS] = READERR;
        }
        break;

    case WRITEBLK:
        // locates target coordinates
        head = (reg[COMMAND] >> HWORDLEN) & BYTEMASK;
        sect = (reg[COMMAND] >> BYTELEN) & BYTEMASK;
        if (isWorking) {
            blkOfs = (diskOfs +
                      ((currCyl * diskP->getHeadNum() * diskP->getSectNum()) +
                       (head * diskP->getSectNum()) + sect) * BLOCKSIZE) * WORDLEN;
            if (diskBuf->WriteBlock(diskFile, blkOfs)) {
                // error writing block to disk file
                sprintf(strbuf, "Unable to write disk %u file : invalid/corrupted file", devNum);
                Panic(strbuf);
            }
            // else all is ok: buffer is still valid
            sprintf(statStr, "C/H/S 0x%.4X/0x%.2X/0x%.2X block written : waiting for ACK",
                    currCyl, head, sect);
            reg[STATUS] = READY;
        } else {
            // error simulation & buffer invalidation 
            cylBuf = headBuf = sectBuf = MAXWORDVAL;
            sprintf(statStr, "Error writing C/H/S 0x%.4X/0x%.2X/0x%.2X : waiting for ACK",
                    currCyl, head, sect);
            reg[STATUS] = WRITERR;
        }
        break;

    default:
        Panic("Unknown operation in DiskDevice::CompleteDevOp()");
        break;
    }

    emit SignalStatusChanged(getDevSStr());
    bus->IntReq(intL, devNum);
    return STATUS;
}


// TapeDevice class allows to emulate removable cartridge tape drives. 
// Individual tapes may be loaded and unloaded, rewound and read 
// (see performance figures shown before). TapeDevice uses the same
// interface as Device, redefining only a few methods' implementation: refer
// to it for individual methods descriptions.
// It adds to Device data structure:
// a pointer to the configuration object containing tape cartridge log
// file name; a static buffer for device operation & status
// description; a FILE structure for log file access; a Block object
// for file handling.

TapeDevice::TapeDevice(systemBus* bus, const MachineConfig* config,
                       unsigned int line, unsigned int devNo)
    : Device(bus, line, devNo),
      config(config)
{
    dType = TAPEDEV;
    isWorking = true;
    tapeFName = NULL;
    tapeBlk = new Block();
    tapeBp = 0;
    tapeLoaded = false;
    reg[STATUS] = READY;
    reg[DATA1] = TAPEEOT;

    sprintf(statStr, "Idle");

    // return value is ignored: a tape will be loaded if its setup file
    // name exists and file is of correct type
    TapeLoad(config->getDeviceFile(line, devNo).c_str());
}

TapeDevice::~TapeDevice()
{
    delete tapeBlk;

    if (tapeLoaded) {
        delete tapeFName;
        
        if (fclose(tapeFile) == EOF) {
            sprintf(strbuf, "Cannot close tape file %u : %s", devNum, strerror(errno));
            Panic(strbuf);
        }
    }
}

// If tFName == NULL or EMPTYSTR, method returns TRUE if 
// a new tape may be loaded,  FALSE otherwise; else, if tFName != NULL
// it tries to load the specified tape file, returning completion status 
bool TapeDevice::TapeLoad(const char* tFName)
{
    Word tapeid = 0;

    if (tFName == NULL || strlen(tFName) == 0) {
        // only a request for tape loading availability
        return !tapeLoaded || (reg[STATUS] == READY && reg[DATA1] == TAPESTART);
    } else {
        if (tapeLoaded && (reg[STATUS] != READY || reg[DATA1] != TAPESTART)) {
            // a new tape cannot be loaded until the current tape is not 
            // rewound: a safeguard to avoid tape ejection during I/O operations
            return false;
        } else {
            if (tapeLoaded) {
                // a tape is currently loaded
                delete tapeFName;
                if (fclose(tapeFile) == EOF) {
                    sprintf(strbuf, "Cannot close tape file %u : %s", devNum, strerror(errno));
                    Panic(strbuf);
                }
                // else all is OK
            }
            tapeFName = new char [strlen(tFName) + 1];
            strcpy(tapeFName, tFName);
            if ((tapeFile = fopen(tapeFName, "r")) == NULL ||
                fread((void *) &tapeid, WORDLEN, 1, tapeFile) != 1 ||
                tapeid != TAPEFILEID)
            {
                sprintf(strbuf, "Cannot open tape %u file : invalid/corrupted file", devNum);
                Panic(strbuf);
            }
            // else file opening is OK: setting tape parameters
            reg[DATA1] = TAPESTART;
            tapeLoaded = true;
            tapeBp = 0;
            emit SignalStatusChanged(getDevSStr());
            return true;
        }
    }
}

void TapeDevice::WriteDevReg(unsigned int regnum, Word data)
{
    // Only COMMAND and DATA0 registers are writable, and only when 
    // device is not busy and a tape is loaded.
    if (!tapeLoaded || reg[STATUS] == BUSY)
        return;

    switch (regnum) {
    case COMMAND:
        // decode operation requested: for each, acknowledges a
        // previous interrupt if pending, sets the device registers,
        // and inserts an Event in SystemBus mantained queue

        reg[COMMAND] = data;

        switch (data) {
        case RESET:
            // it rewinds the tape too
            bus->IntAck(intL, devNum);
            complTime = scheduleIOEvent((TAPERESETTIME + (REWBLKTIME * tapeBp)) * config->getClockRate());
            sprintf(statStr, "Rewinding the tape (last op: %s)", isSuccess(dType, reg[STATUS]));
            reg[STATUS] = BUSY;
            break;

        case ACK:
            bus->IntAck(intL, devNum);
            sprintf(statStr, "Idle (last op: %s)", isSuccess(dType, reg[STATUS]));
            reg[STATUS] = READY;
            break;

        case SKIPBLK:
            bus->IntAck(intL, devNum);
            if (reg[DATA1] != TAPEEOT) {
                sprintf(statStr, "Skipping block %u (last op: %s)", tapeBp, isSuccess(dType, reg[STATUS]));
                complTime = scheduleIOEvent(SKIPBLKTIME * config->getClockRate());
                reg[STATUS] = BUSY;
            } else {
                sprintf(statStr, "Cannot skip beyond EOT : waiting for ACK");
                reg[STATUS] = SKIPERR;
                bus->IntReq(intL, devNum);
            }
            break;

        case READBLK:
            bus->IntAck(intL, devNum);
            if (reg[DATA1] != TAPEEOT) {
                sprintf(statStr, "Reading block %u (last op: %s)", tapeBp, isSuccess(dType, reg[STATUS]));
                complTime = scheduleIOEvent(READBLKTIME * config->getClockRate() + DMATICKS);
                reg[STATUS] = BUSY;
            } else {
                sprintf(statStr, "Cannot read beyond EOT : waiting for ACK");
                reg[STATUS] = READERR;
                bus->IntReq(intL, devNum);
            }
            break;

        case BACKBLK:
            bus->IntAck(intL, devNum);
            // overkill test
            if (reg[DATA1] != TAPESTART && tapeBp != 0) {
                sprintf(statStr, "Rewinding to block %u (last op: %s)", tapeBp - 1, isSuccess(dType, reg[STATUS]));
                complTime = scheduleIOEvent(REWBLKTIME * config->getClockRate());
                reg[STATUS] = BUSY;
            } else {
                sprintf(statStr, "Cannot rewind beyond tape start : waiting for ACK");
                reg[STATUS] = BACKERR;
                bus->IntReq(intL, devNum);
            }
            break;

        default:
            sprintf(statStr, "Unknown command (last op: %s)", isSuccess(dType, reg[STATUS]));
            reg[STATUS] = ILOPERR;
            bus->IntReq(intL, devNum);
            break;
        }

        emit SignalStatusChanged(getDevSStr());
        break;                                       

    case DATA0:
        // sets physical address for buffer in memory
        reg[DATA0] = data;
        break;

    default:
        break;
    }
}

const char* TapeDevice::getDevSStr()
{
    return statStr;
}

unsigned int TapeDevice::CompleteDevOp()
{
    // checks which operation must be completed: for each, sets device
    // register, performs requested operation and produces an interrupt
    // request
    switch (reg[COMMAND]) {
    case RESET:
        // a reset always works, even if isWorking == FALSE
        sprintf(statStr, "Rewind completed : waiting for ACK");
        reg[STATUS] = READY;
        reg[DATA1] = TAPESTART;
        tapeBp = 0;
        break;

    case SKIPBLK:
        // a SKIPBLK is always successful (isWorking status does not matter)
        if (tapeBlk->ReadBlock(tapeFile, (tapeBp * BLOCKSIZE * WORDLEN) + ((tapeBp + 1) * WORDLEN)) || \
            fread((void *) &(reg[DATA1]), WORDLEN, 1, tapeFile) != 1 || reg[DATA1] > TAPEEOB)
        {
            sprintf(strbuf, "Error reading tape %u file : %s", devNum, strerror(errno));
            Panic(strbuf);
        }
        // else operation is successful: 
        reg[STATUS] = READY;
        sprintf(statStr, "Block %u skipped : waiting for ACK", tapeBp);
        tapeBp++;
        break;

    case READBLK:
        if (tapeBlk->ReadBlock(tapeFile, (tapeBp * BLOCKSIZE * WORDLEN) + ((tapeBp + 1) * WORDLEN)) ||
            fread((void *) &(reg[DATA1]), WORDLEN, 1, tapeFile) != 1 || reg[DATA1] > TAPEEOB)
        {
            sprintf(strbuf, "Error reading tape %u file : %s", devNum, strerror(errno));
            Panic(strbuf);
        }
        // else read operation is successful:
        if (isWorking) {
            if (bus->DMATransfer(tapeBlk, reg[DATA0], true)) {
                // DMA transfer error
                reg[STATUS] = DMAERR;
                sprintf(statStr, "DMA error reading block %u : waiting for ACK", tapeBp);
            } else {
                sprintf(statStr, "Block %u read : waiting for ACK", tapeBp);
                reg[STATUS] = READY;
            }
        } else {
            // no operation & error simulation
            sprintf(statStr, "Error reading block %u : waiting for ACK", tapeBp);
            reg[STATUS] = READERR;
        }
        // tape block counter is however incremented
        tapeBp++;
        break;

    case BACKBLK:
        // a BACKBLK is always successful (isWorking status does not matter)
        tapeBp--;
        if (tapeBp == 0)
            reg[DATA1] = TAPESTART;
        else
            // read previous block for terminator value 
            if (tapeBlk->ReadBlock(tapeFile, ((tapeBp - 1) * BLOCKSIZE * WORDLEN) + (tapeBp * WORDLEN)) ||
                fread((void *) &(reg[DATA1]), WORDLEN, 1, tapeFile) != 1 || reg[DATA1] > TAPEEOB)
            {
                sprintf(strbuf, "Error reading tape %u file : %s", devNum, strerror(errno));
                Panic(strbuf);
            }
        // else operation is successful:
        reg[STATUS] = READY;
        sprintf(statStr, "Rewound back to block %u : waiting for ACK", tapeBp);
        break;

    default:
        Panic("Unknown operation in TapeDevice::CompleteDevOp()");
        break;
    }

    emit SignalStatusChanged(getDevSStr());
    bus->IntReq(intL, devNum);

    // here Reg[DATA1] too is changed, but there is only one return value
    // however, if area is traced/suspected fully the result does not change
    return STATUS;
}

/****************************************************************************/
/* Definitions strictly local to the module.                                */
/****************************************************************************/

// This function decodes device STATUS field and tells if previous operation
// has been successful or not
HIDDEN const char * isSuccess(unsigned int devType, Word regVal)
{
    const char * result = NULL;
        
    switch (devType) {
    case PRNTDEV:
    case DISKDEV:
    case TAPEDEV:
    case ETHDEV:
        if (regVal == READY)
            result = opResult[true];
        else
            result = opResult[false];
        break;
                
    case TERMDEV:
        if (regVal == READY || regVal == RECVD || regVal == TRANSMD)
            result = opResult[true];
        else
            result = opResult[false];
        break;
                
    default:
        Panic("Unknown device in device module::isSuccess()");
        break;
    }
    return(result);
}       


// EthDevice class allows to emulate an ethernet interface

EthDevice::EthDevice(systemBus* bus, const MachineConfig* cfg, unsigned int line, unsigned int devNo)
    : Device(bus, line, devNo),
      config(cfg)
{
    // adds to a Device object EthDevice-specific fields
    dType = ETHDEV;
    isWorking = true;
    reg[STATUS] = READY;

    readbuf = new Block();
    writebuf = new Block();
    sprintf(statStr, "Idle");

    // FIXME: we should make this much better (and hairy...)
    if (!testnetinterface(config->getDeviceFile(intL, devNum).c_str()))
        throw EthError(devNo);

    /* open the net */
    netint = new netinterface(config->getDeviceFile(intL, devNum).c_str(),
                              (const char*) config->getMACId(devNum),
                              devNum);
    if (netint->getmode() & INTERRUPT) {
        scheduleIOEvent(POLLNETTIME * config->getClockRate());
        polling = true;
    } else {
        polling = false;
    }
}

EthDevice::~EthDevice()
{
    delete netint;
    delete readbuf;
    delete writebuf;
}

void EthDevice::WriteDevReg(unsigned int regnum, Word data)
{
    int rp = reg[STATUS] & READPENDING;
    int err = 0;

    if ((reg[STATUS] & READPENDINGMASK) != BUSY) {
        switch (regnum) {
        case COMMAND:
            // decode operation requested: for each, acknowledges a
            // previous interrupt if pending, sets the device registers,
            // and inserts an Event in SystemBus mantained queue
            reg[COMMAND] = data;
            switch (data) {
            case RESET:
                bus->IntAck(intL, devNum);
                sprintf(statStr, "Reset requested : waiting for ACK");
                reg[STATUS] = BUSY;
                complTime = scheduleIOEvent(ETHRESETTIME * config->getClockRate());
                break;
            case ACK:
                bus->IntAck(intL, devNum);
                sprintf(statStr, "Idle (last op: %s)", isSuccess(dType, reg[STATUS] & READPENDINGMASK));
                reg[STATUS] = READY;
                break;
            case READCONF:
                bus->IntAck(intL, devNum);
                reg[STATUS] = BUSY;
                sprintf(statStr, "Reading Interface Configuration");
                complTime = scheduleIOEvent(CONFNETTIME * config->getClockRate());
                break;
            case CONFIGURE:
                bus->IntAck(intL, devNum);
                reg[STATUS] = BUSY;
                sprintf(statStr, "Writing Interface Configuration");
                complTime = scheduleIOEvent(CONFNETTIME * config->getClockRate());
                break;
            case READNET:
                bus->IntAck(intL, devNum);
                reg[STATUS] = BUSY;
                complTime = scheduleIOEvent(READNETTIME * config->getClockRate());
                sprintf(statStr, "Receiving Data");
                break;
            case WRITENET:
                bus->IntAck(intL, devNum);
                if (bus->DMAVarTransfer(writebuf, reg[DATA0], reg[DATA1], false)) {
                    reg[STATUS] = DMAERR;
                    sprintf(statStr, "DMA error on netwrite: waiting for ACK");
                    err=1;
                } else {
                    complTime = scheduleIOEvent(WRITENETTIME * config->getClockRate());
                    reg[STATUS] = BUSY;
                    sprintf(statStr, "Sending Data");
                }
                break;
            default:
                sprintf(statStr, "Unknown command (last op: %s)", isSuccess(dType, reg[STATUS] & READPENDINGMASK));
                reg[STATUS] = ILOPERR;
                err=1;
                break;
            }
            reg[STATUS] |= rp;
            if (err)
                bus->IntReq(intL, devNum);
            emit SignalStatusChanged(getDevSStr());
            break;

        case DATA0:
            reg[DATA0] = data;
            break;

        case DATA1:
            reg[DATA1] = data;
            break;

        default:
            break;
        }
    }
}

const char* EthDevice::getDevSStr()
{
    return statStr;
}

unsigned int EthDevice::CompleteDevOp()
{
    int rp = reg[STATUS] & READPENDING;
    const bool busy = (reg[STATUS] & READPENDINGMASK) == BUSY;

    if (polling && !busy) {
        /* polling with no pending ops */
        polling = false;
        if (!rp) {
            /* process has not been informed yet */
            if (netint->polling()) {
                /* there are waiting packets */
                reg[STATUS] = reg[STATUS] | READPENDING;
                emit SignalStatusChanged(getDevSStr());
                bus->IntReq(intL, devNum);
            } else {
                /* there are no waiting packets;
                   continue polling if the user hasn't changed her mind */
                if (netint->getmode() & INTERRUPT) {
                    scheduleIOEvent(POLLNETTIME * config->getClockRate());
                    polling = true;
                }
            }
        }
    } else {
        //Real operation
        switch (reg[COMMAND]) {
        case RESET:
            // a reset always works, even if isWorking == FALSE
            sprintf(statStr, "Reset completed : waiting for ACK");
            reg[STATUS] = READY;
            break;
        case READCONF:
            // readconf always works even if isWorking == FALSE
        {
            char macaddr[6];
            sprintf(statStr, "Interface Configuration Read : waiting for ACK");
            netint->getaddr(macaddr);
            reg[DATA0]=(((Word) netint->getmode()) <<16) | (((Word) macaddr[0])<<8) | ((Word) macaddr[1]);
            reg[DATA1]=((Word) macaddr[2])<<24 | ((Word) macaddr[3])<<16 | ((Word) macaddr[4]) <<8 | ((Word)macaddr[5]); 
        }
        reg[STATUS] = READY;
        break;
        case CONFIGURE: 
            // configure always works even if isWorking == FALSE
        {
            char macaddr[6];
            int newmode=reg[DATA0]>>16;
            if ((newmode & SETMAC) != 0) {
                macaddr[0]=reg[DATA0]>>8 & 0xff;
                macaddr[1]=reg[DATA0] & 0xff;
                macaddr[2]=reg[DATA1]>>24 & 0xff;
                macaddr[3]=reg[DATA1]>>16 & 0xff;
                macaddr[4]=reg[DATA1]>>8 & 0xff;
                macaddr[5]=reg[DATA1] & 0xff;
                netint->setaddr(macaddr);
            }
            newmode &= ~SETMAC;
            sprintf(statStr, "Interface Reconfigured: waiting for ACK");
            netint->setmode(newmode); 
        }
        reg[STATUS] = READY;
        break;
        case READNET:
            if (isWorking)
            {
                if ((reg[DATA1]=netint->readdata((char *) readbuf, PACKETSIZE)) < 0) {
                    sprintf(statStr, "Net reading error: waiting for ACK");
                    reg[STATUS] = READERR;
                } else if (reg[DATA1] == 0) {
                    sprintf(statStr, "No pending packet for read: waiting for ACK");
                    reg[STATUS] = READY;
                } else {
                    if (bus->DMAVarTransfer(readbuf, reg[DATA0], reg[DATA1], true)) {
                        reg[STATUS] = DMAERR;
                        sprintf(statStr, "DMA error on netread: waiting for ACK");
                    } else {
                        sprintf(statStr, "Packet received: waiting for ACK");
                        reg[STATUS] = READY;
                    }
                }
                rp=netint->polling();
            }
            else
            {
                // no operation & error simulation
                sprintf(statStr, "Net reading error : waiting for ACK");
                reg[STATUS] = READERR;
            }				
            break;
        case WRITENET:
            if (isWorking)
            {
                if (reg[DATA1] == netint->writedata((char *)writebuf, reg[DATA1])) 
                {
                    sprintf(statStr, "Packet Sent: waiting for ACK");
                    reg[STATUS] = READY;
                } 
                else 
                {
                    sprintf(statStr, "Net writing error: waiting for ACK");
                    reg[STATUS] = WRITERR;
                }
            }
            else
            {
                // no operation & error simulation
                sprintf(statStr, "Net writing error : waiting for ACK");
                reg[STATUS] = WRITERR;
            }
            break;
        }

        emit SignalStatusChanged(getDevSStr());
        reg[STATUS] |= rp;
        bus->IntReq(intL, devNum);

        // If user wants interrupts, we are not already polling, and
        // there are no pending read requests, schedule another poll
        // event.
        if (netint->getmode() & INTERRUPT && !polling && !rp) {
            scheduleIOEvent(POLLNETTIME * config->getClockRate());
            polling = true;
        }
    }

    return STATUS;
}

bool EthDevice::isBusy() const
{
    return (reg[STATUS] & READPENDINGMASK) == BUSY;
}

#endif //UARM_DEVICE_CC
