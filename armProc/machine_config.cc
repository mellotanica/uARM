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

#ifndef UARM_MACHINE_CONFIG_CC
#define UARM_MACHINE_CONFIG_CC

#include "armProc/machine_config.h"

#include <config.h>

#include <iostream>
#include <cstdio>
#include <fstream>
#include <cassert>
#include <algorithm>

#include <boost/format.hpp>

#include "armProc/const.h"
#include "services/json.h"
#include "services/error.h"
#include "services/utility.h"
#include "services/util.h"


// STATIC: PACKAGE_DATA_DIR deve essere ereditata dal compilatore! (guardare Makefile in umps/src/umps/)
#define PACKAGE_DATA_DIR "facilities";

template<typename T>
static T bumpProperty(T minValue, T value, T maxValue)
{
    if (value < minValue)
        return minValue;
    else if (value > maxValue)
        return maxValue;
    else
        return value;
}

const char* const MachineConfig::deviceKeyPrefix[N_EXT_IL] = {
    "disk",
    "tape",
    "eth",
    "printer",
    "terminal"
};

MachineConfig* MachineConfig::LoadFromFile(const std::string& fileName, std::string& error, QApplication *app)
{
    std::ifstream inputStream(fileName.c_str());
    if (inputStream.fail()) {
        error = boost::str(boost::format("Unable to open file `%s'") %fileName);
        return NULL;
    }

    std::auto_ptr<JsonObject> root;

    try {
        JsonParser parser;
        JsonNode* node = parser.Parse(inputStream);
        if (!node->Holds(JSON_OBJECT)) {
            error = "Invalid machine configuration file (object expected)";
            return NULL;
        }
        root.reset(node->AsObject());
    } catch (JsonParser::SyntaxError& e) {
        error = "Invalid machine configuration file (erroneous JSON syntax)";
        return NULL;
    }

    std::auto_ptr<MachineConfig> config(new MachineConfig(fileName, app));

    try {
        if (root->HasMember("num-processors"))
            config->setNumProcessors(root->Get("num-processors")->AsNumber());
        if (root->HasMember("clock-rate"))
            config->setClockRate(root->Get("clock-rate")->AsNumber());
        if (root->HasMember("num-ram-frames"))
            config->setRamSize(root->Get("num-ram-frames")->AsNumber());

        if (root->HasMember("boot")) {
            JsonObject* bootOpt = root->Get("boot")->AsObject();
            config->setLoadCoreEnabled(bootOpt->Get("load-core-file")->AsBool());
            config->setROM(ROM_TYPE_CORE, bootOpt->Get("core-file")->AsString());
        }

        if (root->HasMember("execution-rom"))
            config->setROM(ROM_TYPE_BIOS, root->Get("execution-rom")->AsString());

        if (root->HasMember("symbol-table")) {
            JsonObject* stab = root->Get("symbol-table")->AsObject();
            config->setROM(ROM_TYPE_STAB, stab->Get("file")->AsString());
            config->setSymbolTableASID(stab->Get("asid")->AsNumber());
        }

        if (root->HasMember("devices")) {
            JsonObject* devices = root->Get("devices")->AsObject();
            for (unsigned int il = 0; il < N_EXT_IL; il++) {
                for (unsigned int devNo = 0; devNo < N_DEV_PER_IL; devNo++) {
                    std::string key = boost::str(boost::format("%s%u") %deviceKeyPrefix[il] %devNo);
                    if (devices->HasMember(key)) {
                        JsonObject* devObj = devices->Get(key)->AsObject();
                        config->setDeviceEnabled(il, devNo, devObj->Get("enabled")->AsBool());
                        config->setDeviceFile(il, devNo, devObj->Get("file")->AsString());
                        if (il == EXT_IL_INDEX(IL_ETHERNET) && devObj->HasMember("address")) {
                            uint8_t macId[6];
                            if (ParseMACId(devObj->Get("address")->AsString(), macId))
                                config->setMACId(devNo, macId);
                        }
                    }
                }
            }
        }
    } catch (JsonNode::JsonError& e) {
        error = "Invalid machine configuration file";
        return NULL;
    }

    return config.release();
}

MachineConfig* MachineConfig::Create(const std::string& fileName, QApplication *app)
{
    std::auto_ptr<MachineConfig> config(new MachineConfig(fileName, app));

    // The constructor initializes all the basic fields to sane
    // initial values; in addition, we enable a terminal device for
    // newly created configs.
    config->setDeviceFile(EXT_IL_INDEX(IL_TERMINAL), 0, "term0.umps");
    config->setDeviceEnabled(EXT_IL_INDEX(IL_TERMINAL), 0, true);

    config->Save();

    return config.release();
}

void MachineConfig::Save()
{
    scoped_ptr<JsonObject> root(new JsonObject);

    root->Set("num-processors", (int) getNumProcessors());
    root->Set("clock-rate", (int) getClockRate());
    root->Set("tlb-size", (int) getTLBSize());
    root->Set("num-ram-frames", (int) getRamSize());

    JsonObject* bootOpt = new JsonObject;
    bootOpt->Set("load-core-file", isLoadCoreEnabled());
    bootOpt->Set("core-file", getROM(ROM_TYPE_CORE));
    root->Set("boot", bootOpt);

    root->Set("execution-rom", getROM(ROM_TYPE_BIOS));

    JsonObject* stabObject = new JsonObject;
    stabObject->Set("file", romFiles[ROM_TYPE_STAB]);
    stabObject->Set("asid", (int) symbolTableASID);
    root->Set("symbol-table", stabObject);

    JsonObject* devicesObject = new JsonObject;
    for (unsigned int il = 0; il < N_EXT_IL; il++) {
        for (unsigned int devNo = 0; devNo < N_DEV_PER_IL; devNo++) {
            if (!devFiles[il][devNo].empty()) {
                JsonObject* object = new JsonObject;
                object->Set("enabled", devEnabled[il][devNo]);
                object->Set("file", devFiles[il][devNo]);
                if (il == EXT_IL_INDEX(IL_ETHERNET) && getMACId(devNo))
                    object->Set("address", MACIdToString(getMACId(devNo)));
                std::string key = boost::str(boost::format("%s%u") %deviceKeyPrefix[il] %devNo);
                devicesObject->Set(key, object);
            }
        }
    }
    root->Set("devices", devicesObject);

    std::string buf;
    root->Serialize(buf, true);

    std::ofstream file(fileName.c_str(), std::ios_base::trunc | std::ios_base::out);
    if (file.fail() || !(file << buf))
        throw FileError(fileName);
    file.flush();
}

MachineConfig::MachineConfig(const std::string& fn, QApplication *app)
    : fileName(fn),
      app(app)
{
    resetToFactorySettings();
}

bool MachineConfig::Validate(std::list<std::string>* errors) const
{
    bool isValid = true;
    if (romFiles[ROM_TYPE_BIOS].empty()) {
        if (errors)
            errors->push_back("BIOS ROM file not set");
        isValid = false;
    }
    if (romFiles[ROM_TYPE_STAB].empty()) {
        if (errors)
            errors->push_back("Symbol table file not set");
        isValid = false;
    }
    return isValid;
}

void MachineConfig::setRamSize(Word size)
{
    ramSize = bumpProperty(MIN_RAM, size, MAX_RAM);
}

void MachineConfig::setNumProcessors(unsigned int value)
{
    cpus = bumpProperty(MIN_CPUS, value, MAX_CPUS);
}

void MachineConfig::setClockRate(unsigned int value)
{
    clockRate = bumpProperty(MIN_CLOCK_RATE, value, MAX_CLOCK_RATE);
}

void MachineConfig::setROM(ROMType type, const std::string& fileName)
{
    romFiles[type] = fileName;
}

const std::string& MachineConfig::getROM(ROMType type) const
{
    return romFiles[type];
}

void MachineConfig::setSymbolTableASID(Word asid)
{
    symbolTableASID = bumpProperty(MIN_ASID, asid, MAX_ASID);
}

unsigned int MachineConfig::getDeviceType(unsigned int il, unsigned int devNo) const
{
    assert(il < N_EXT_IL && devNo < N_DEV_PER_IL);

    static unsigned int types[] = {
        DISKDEV, TAPEDEV, ETHDEV, PRNTDEV, TERMDEV
    };

    if (getDeviceEnabled(il, devNo) && !getDeviceFile(il, devNo).empty())
        return types[il];
    else
        return NULLDEV;
}

bool MachineConfig::getDeviceEnabled(unsigned int il, unsigned int devNo) const
{
    assert(il < N_EXT_IL && devNo < N_DEV_PER_IL);
    return devEnabled[il][devNo];
}

void MachineConfig::setDeviceEnabled(unsigned int il, unsigned int devNo, bool setting)
{
    assert(il < N_EXT_IL && devNo < N_DEV_PER_IL);
    devEnabled[il][devNo] = setting;
}

void MachineConfig::setDeviceFile(unsigned int il, unsigned int devNo, const std::string& fileName)
{
    assert(il < N_EXT_IL && devNo < N_DEV_PER_IL);
    devFiles[il][devNo] = fileName;
}

const std::string& MachineConfig::getDeviceFile(unsigned int il, unsigned int devNo) const
{
    assert(il < N_EXT_IL && devNo < N_DEV_PER_IL);
    return devFiles[il][devNo];
}

const uint8_t* MachineConfig::getMACId(unsigned int devNo) const
{
    assert(devNo < N_DEV_PER_IL);
    return macId[devNo].get();
}

void MachineConfig::setMACId(unsigned int devNo, const uint8_t* value)
{
    assert(devNo < N_DEV_PER_IL);

    if (value != NULL) {
        macId[devNo].reset(new uint8_t[6]);
        std::copy(value, value + 6, macId[devNo].get());
    } else {
        macId[devNo].reset();
    }
}

void MachineConfig::resetToFactorySettings()
{
    setNumProcessors(DEFAULT_NUM_CPUS);
    setClockRate(DEFAULT_CLOCK_RATE);
    setRamSize(DEFAUlT_RAM_SIZE);

    std::string dataDir = app->applicationDirPath().toStdString() + "/facilities";

    // STATIC: this is a temp bios, there needs to be a more complete one..
    setROM(ROM_TYPE_BIOS, dataDir + "/BIOS.rom.uarm");

    setLoadCoreEnabled(true);
    setROM(ROM_TYPE_CORE, "kernel.core.uarm");
    setROM(ROM_TYPE_STAB, "kernel.stab.uarm");
    setSymbolTableASID(MAX_ASID);

    for (unsigned int i = 0; i < N_EXT_IL; ++i)
        for (unsigned int j = 0; j < N_DEV_PER_IL; ++j)
            devEnabled[i][j] = false;
}

bool MachineConfig::validFileMagic(Word tag, const char* fName)
{
    bool valid;
    FILE* file = NULL;

    if (tag == 0UL) {
        file = fopen(fName, "a");
        valid = (file != NULL);
    } else {
        file = fopen(fName, "r");
        Word fileTag;
        valid = ((file != NULL) &&
                 (fread((void*) &fileTag, WS, 1, file) == 1) &&
                 (fileTag == tag));
    }

    if (file != NULL)
        fclose(file);
    return valid;
}

MC_Holder* MC_Holder::instance = NULL;
MachineConfig* MC_Holder::config = NULL;

MC_Holder* MC_Holder::getInstance(){
    if(instance == NULL)
        instance = new MC_Holder();
    return instance;
}

#endif //UARM_MACHINE_CONFIG_CC
