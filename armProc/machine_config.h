/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uMPS - A general purpose computer system simulator
 *
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

#ifndef UARM_MACHINE_CONFIG_H
#define UARM_MACHINE_CONFIG_H

#include <string>
#include <list>

#include "services/lang.h"
#include "armProc/basic_types.h"
#include "facilities/arch.h"
#include "armProc/types.h"
#include "qarm/guiConst.h"
#include "facilities/uARMconst.h"

#include <QApplication>

class qarm;

enum ROMType {
    ROM_TYPE_BIOS,
    ROM_TYPE_CORE,
    ROM_TYPE_STAB,
    N_ROM_TYPES
};

typedef struct {
    unsigned int monofont_size;
} main_configs_s;

class MachineConfig {
public:
    static const Word MIN_RAM = 8;
    static const Word MAX_RAM = 1024;
    static const Word DEFAULT_RAM_SIZE = 256;

    static const Word MIN_TLB_SIZE = 4;
    static const Word MAX_TLB_SIZE = 64;
    static const Word DEFAULT_TLB_SIZE = 16;

    // STATIC: only one cpu...
    static const unsigned int MIN_CPUS = 1;
    static const unsigned int MAX_CPUS = 1;
    static const unsigned int DEFAULT_NUM_CPUS = 1;

    static const unsigned int MIN_CLOCK_RATE = 1;
    static const unsigned int MAX_CLOCK_RATE = 50;
    static const unsigned int DEFAULT_CLOCK_RATE = 1;

    static const unsigned int MIN_REFRESH_RATE = 1;
    static const unsigned int MAX_REFRESH_RATE = 1000;
    static const unsigned int DEFAULT_REFRESH_RATE = 600;

    static const bool DEFAULT_REFRESH_ON_PAUSE = false;

    static const bool DEFAULT_STOP_ON_EXCEPTION = false;
    static const bool DEFAULT_STOP_ON_TLB_CHANGE = false;

    static const bool DEFAULT_ACCESSIBLE_MODE = false;

    static const Word MIN_ASID = 0;
    static const Word MAX_ASID = MAXASID-1;

    static MachineConfig* LoadFromFile(const std::string& fileName, std::string& error, QApplication *app, qarm *widget);
    static MachineConfig* Create(const std::string& fileName, const std::string& homedir, QApplication *app, qarm *widget);

    const std::string& getFileName() const { return fileName; }

    void Save();

    bool Validate(std::list<std::string>* errors) const;

    // STATIC: now only direct core loading is possible.. change this!
    //void setLoadCoreEnabled(bool setting) { loadCoreFile = setting; }
    void setLoadCoreEnabled(bool setting) { loadCoreFile = setting | true; }
    bool isLoadCoreEnabled() const { return loadCoreFile; }

    void setStopOnException(bool setting) { stopOnException = setting; }
    bool getStopOnException() const { return stopOnException; }

    void setRamSize(Word size);
    Word getRamSize() const { return ramSize; }

    void setNumProcessors(unsigned int value);
    unsigned int getNumProcessors() const { return cpus; }

    void setClockRate(unsigned int value);
    unsigned int getClockRate() const { return clockRate; }

    void setRefreshRate(unsigned int value);
    unsigned int getRefreshRate() const { return refreshRate; }

    void setRefreshOnPause(bool enabled);
    bool getRefreshOnPause() const { return refreshOnPause; }

    void setTLBSize(Word size);
    Word getTLBSize() const { return tlbSize; }

    void setStopOnTLBChange(bool value);
    bool getStopOnTLBChange() const { return stopOnTLBChange; }

    void setROM(ROMType type, const std::string& fileName);
    const std::string& getROM(ROMType type) const;

    void setSymbolTableASID(Word asid);
    Word getSymbolTableASID() const { return symbolTableASID; }

    void setAccessibleMode(bool enabled);
    bool getAccessibleMode() const { return accessibleMode; }

    unsigned int getDeviceType(unsigned int il, unsigned int devNo) const;
    bool getDeviceEnabled(unsigned int il, unsigned int devNo) const;
    void setDeviceEnabled(unsigned int il, unsigned int devNo, bool setting);
    void setDeviceFile(unsigned int il, unsigned int devNo, const std::string& fileName);
    const std::string& getDeviceFile(unsigned int il, unsigned int devNo) const;
    const uint8_t* getMACId(unsigned int devNo) const;
    void setMACId(unsigned int devNo, const uint8_t* value);

    QApplication* getApp() {return app;}
    QString getAppPath() {return app->applicationDirPath();}

    qarm *getMainWidget() {return mainWidget;}

private:
    MachineConfig(const std::string& fileName, QApplication *app, qarm *widget);

    void resetToFactorySettings();
    bool validFileMagic(Word tag, const char* fName);

    std::string fileName;

    bool loadCoreFile;
    bool refreshOnPause;
    bool stopOnException;
    bool stopOnTLBChange;

    bool accessibleMode;

    Word ramSize;
    unsigned int cpus;
    unsigned int clockRate;
    unsigned int refreshRate;
    Word tlbSize;

    std::string romFiles[N_ROM_TYPES];
    Word symbolTableASID;

    std::string devFiles[N_EXT_IL][N_DEV_PER_IL];
    bool devEnabled[N_EXT_IL][N_DEV_PER_IL];
    scoped_array<uint8_t> macId[N_DEV_PER_IL];

    QApplication *app;
    qarm *mainWidget;

    static const char* const deviceKeyPrefix[N_EXT_IL];
};

class MC_Holder {
private:
    static MC_Holder *instance;
    static MachineConfig *config;

    MC_Holder() {}

public:
    ~MC_Holder() {}
    static MC_Holder* getInstance();

    MachineConfig *getConfig() {return config;}
    void setConfig(MachineConfig *conf) {config = conf;}

    main_configs_s mainConfigs;

    // FIXME: ugly debug support
    bool dumpExecution;
    char* dumpFilename;
};

#endif // UARM_MACHINE_CONFIG_H
