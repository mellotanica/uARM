/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uMPS - A general purpose computer system simulator
 *
 * Copyright (C) 2004 Mauro Morsiani
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

#ifndef UMPS_DEVICE_H
#define UMPS_DEVICE_H

#include <QObject>
#include "armProc/types.h"
#include "armProc/const.h"
#include "armProc/bus.h"

enum DeviceType {
    DT_NULL = 0,
    DT_DISK,
    DT_TAPE,
    DT_ETH,
    DT_PRINTER,
    DT_TERMINAL,
    N_DEVICES
};

#define PRNTBUFSIZE 128
#define TERMBUFSIZE 128
#define DISKBUFSIZE 128
#define TAPEBUFSIZE 128
#define ETHBUFSIZE 128

class Block;
class DriveParams;
class netinterface;
class MachineConfig;

// Device class defines the interface to all device types, and represents
// the "uninstalled device" (NULLDEV) itself. Device objects are created and
// controlled by a SystemBus object, but also may be inspected by Watch if
// needed

class Device : public QObject{
    Q_OBJECT
signals:
    void SignalStatusChanged(const char* status);
    void SignalConditionChanged(bool cond);
    void SignalTransmitted(char tChar);

public:
    // This method creates a Device object with "coordinates" (interrupt
    // line, device number) clears device register, and links it to bus
    Device(systemBus * busl, unsigned int intl, unsigned int dnum);

    virtual ~Device();

    unsigned int getInterruptLine() const { return intL; }
    unsigned int getNumber() const { return devNum; }

    // This method returns device type ID
    unsigned int Type() const { return dType; }

    // This method is invoked by SystemBus when the required operation
    // scheduled on the device (using Event objects and queue) should be
    // completed: the default NULLDEV device has nothing to do, but
    // others do
    virtual unsigned int CompleteDevOp();

    // This method allows SystemBus to write into device register for
    // device: NULLDEV device register write has no effects, but other
    // devices will start performing required operations if COMMAND
    // register is written with proper codes
    virtual void WriteDevReg(unsigned int regnum, Word data);

    // This method returns the static buffer contained in each device
    // describing the current device status (operation performed, etc.).
    // NULLDEV devices are not operational
    virtual const char* getDevSStr();

    /*
     * Return a human-readable expression for completion time of the
     * current device operation.
     */
    virtual std::string getCTimeInfo() const;

    // This method allows to copy inputstr contents inside
    // TerminalDevice receiver buffer: not operational for all other
    // devices (NULLDEV included) and produces a panic message
    virtual void Input(const char* inputstr);

    // This method allows to load/unload tapes inside a TapeDevice. For
    // it, if tFName == NULL or EMPTYSTR, method returns TRUE if a new
    // tape may be loaded, FALSE otherwise; else, if tFName != NULL it
    // tries to load the specified tape file, returning completion
    // status. This method is not operational for other devices (NULLDEV
    // included) and produces a panic message
    virtual bool TapeLoad(const char * tFName);

    // This method returns the current value for device register field
    // indexed by regnum
    Word ReadDevReg(unsigned int regnum);

    // This method gets the current operational status for the device,
    // as set by user inside the simulation; a "not-working" device
    // fails all operations requested and reports proper error codes; a
    // NULLDEV always fails.
    bool getDevNotWorking();

    // This method sets the operational status for the device inside the
    // simulation as user wishes; a "not-working" device fails all operations
    // requested and reports proper error codes; a NULLDEV always fail
    bool setDevNotWorking(bool cond);

    void setCondition(bool working);
    bool getCondition() const { return isWorking; }

    //WARN: vecchi sig
    //sigc::signal<void, const char*> SignalStatusChanged;
    //sigc::signal<void, bool> SignalConditionChanged;

protected:
    virtual bool isBusy() const;
    uint64_t scheduleIOEvent(uint64_t delay);

    // Interrupt line and device number
    unsigned int intL;
    unsigned int devNum;

    // device register structure
    Word reg[DEVREGLEN];

    // device type ID (see h/const.h)
    unsigned int dType;

    // controlling SystemBus object
    systemBus* bus;

    // Completion time for current operation (if any)
    uint64_t complTime;

    // device operational status
    bool isWorking;
};


/**************************************************************************/


// PrinterDevice class allows to emulate parallel character printer
// currently in use (see performance figures shown before). It uses the same
// interface as Device, redefining only a few methods' implementation: refer
// to it for individual methods descriptions. 
// It adds to Device data structure:
// a pointer to SetupInfo object containing printer log file name;
// a static buffer for device operation & status description;
// a FILE structure for log file access.

class PrinterDevice : public Device {
public:
    PrinterDevice(systemBus* busl, const MachineConfig* config, unsigned int intl, unsigned int dnum);
    virtual ~PrinterDevice();
    virtual void WriteDevReg(unsigned int regnum, Word data);
    virtual unsigned int CompleteDevOp();
    virtual const char* getDevSStr();

private:
    const MachineConfig* const config;

    // log file handling
    FILE * prntFile;

    char statStr[PRNTBUFSIZE];
};


/**************************************************************************/


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

class TerminalDevice : public Device {
public:
    TerminalDevice(systemBus* bus, const MachineConfig* cfg, unsigned int il, unsigned int devNo);
    virtual ~TerminalDevice();

    virtual void WriteDevReg(unsigned int regnum, Word data);
    virtual unsigned int CompleteDevOp();

    virtual const char* getDevSStr();
    const char* getTXStatus() const;
    const char* getRXStatus() const;

    std::string getTXCTimeInfo() const;
    std::string getRXCTimeInfo() const;
    virtual std::string getCTimeInfo() const;

    virtual void Input(const char * inputstr);

    //WARN: vecchio signal
    //sigc::signal<void, char> SignalTransmitted;

private:
    const MachineConfig* const config;

    // for log file handling
    FILE * termFile;

    // receiver buffer and pointer to first character to receive from it
    char * recvBuf;
    unsigned int recvBp;
		
    // static buffer for receiver
    char recvStatStr[TERMBUFSIZE];

    // static buffer for transmitter
    char tranStatStr[TERMBUFSIZE];

    // Completion time for current receiver operation (if any)
    uint64_t recvCTime;

    // Completion time for current transmitter operation (if any)
    uint64_t tranCTime;

    // receiver operation pending flag
    bool recvIntPend;

    // transmitter operation pending flag
    bool tranIntPend;
};


/**************************************************************************/


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
// a pointer to SetupInfo object containing printer log file name;
// a static buffer for device operation & status description;
// a FILE structure for disk image file access;
// a set of disk parameters (read from disk image file header);
// a Block object for file handling;
// some items for performance computation.

class DiskDevice : public Device {
public:
    DiskDevice(systemBus* bus, const MachineConfig* cfg, unsigned int line, unsigned int devNo);
    virtual ~DiskDevice();
    virtual void WriteDevReg(unsigned int regnum, Word data);
    virtual unsigned int CompleteDevOp();
    virtual const char * getDevSStr();

private:
    const MachineConfig* const config;

    // to handle it
    FILE * diskFile;
		
    // static buffer
    char statStr[DISKBUFSIZE];
		
    // sector buffer and coordinates on disk (cyl, head, sect)
    Block * diskBuf;
    unsigned int cylBuf, headBuf, sectBuf;
		
    // start of disk image inside file (after header)
    SWord diskOfs;
		
    // disk performance parameters
    DriveParams * diskP;
		
    // sector underhead time in ticks
    Word sectTicks;

    // current cylinder
    unsigned int currCyl;
};


/**************************************************************************/


// TapeDevice class allows to emulate removable cartridge tape drives. 
// Individual tapes may be loaded and unloaded, rewound and read 
// (see performance figures shown before). TapeDevice uses the same
// interface as Device, redefining only a few methods' implementation: refer
// to it for individual methods descriptions.
// It adds to Device data structure:
// a pointer to SetupInfo object containing tape cartridge log file name;
// a static buffer for device operation & status description;
// a FILE structure for log file access;
// a Block object for file handling.

class TapeDevice : public Device {
public:
    TapeDevice(systemBus* bus, const MachineConfig* cfg, unsigned int line, unsigned int devNo);
    virtual ~TapeDevice();
    virtual void WriteDevReg(unsigned int regnum, Word data);
    virtual unsigned int CompleteDevOp();
    virtual const char * getDevSStr();
    virtual bool TapeLoad(const char * tFName);

private:
    const MachineConfig* const config;

    // to access tape image file
    FILE * tapeFile;
    char * tapeFName;

    // to read tape blocks and know current position (starts with block 0)
    Block * tapeBlk;
    unsigned int tapeBp;

    // loaded cartridge flag
    bool tapeLoaded;

    // static buffer
    char statStr[TAPEBUFSIZE];
};




/**************************************************************************/


// EthDevice class allows to emulate an ethernet interface

class EthDevice : public Device
{ 
public:
    EthDevice(systemBus* bus, const MachineConfig* config, unsigned int line, unsigned int devNo);
    virtual ~EthDevice();
    virtual void WriteDevReg(unsigned int regnum, Word data);
    virtual unsigned int CompleteDevOp();
    virtual const char* getDevSStr();

protected:
    virtual bool isBusy() const;

private:
    const MachineConfig* const config;

    Block *readbuf;
    Block *writebuf;

    // static buffer
    char statStr[ETHBUFSIZE];

    bool polling;

    netinterface *netint;
};

#endif // UMPS_DEVICE_H
