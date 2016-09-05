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

MachineConfig* MachineConfig::LoadFromFile(const std::string& fileName, std::string& error, QApplication *app, qarm *widget)
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

    std::auto_ptr<MachineConfig> config(new MachineConfig(fileName, app, widget));

    try {
        if (root->HasMember("clock-rate"))
            config->setClockRate(root->Get("clock-rate")->AsNumber());
        if (root->HasMember("refresh-rate"))
            config->setRefreshRate(root->Get("refresh-rate")->AsNumber());
        if (root->HasMember("refresh-on-pause"))
            config->setRefreshOnPause(root->Get("refresh-on-pause")->AsBool());
        if (root->HasMember("num-ram-frames"))
            config->setRamSize(root->Get("num-ram-frames")->AsNumber());
        if (root->HasMember("pause-on-exc"))
            config->setStopOnException(root->Get("pause-on-exc")->AsBool());
        if (root->HasMember("pause-on-tlb"))
            config->setStopOnTLBChange(root->Get("pause-on-tlb")->AsBool());
        if (root->HasMember("accessible-mode"))
            config->setAccessibleMode(root->Get("accessible-mode")->AsBool());
        if (root->HasMember("core-file")) {
            config->setROM(ROM_TYPE_CORE, root->Get("core-file")->AsString());
        }

        if (root->HasMember("execution-rom"))
            config->setROM(ROM_TYPE_BIOS, root->Get("execution-rom")->AsString());

        if (root->HasMember("symbol-table")) {
            JsonObject* stab = root->Get("symbol-table")->AsObject();
            if(stab->HasMember("external-stab")){
                config->setExternalStab(stab->Get("external-stab")->AsBool());
            }
            config->setSymbolTableASID(stab->Get("asid")->AsNumber());
            config->setROM(ROM_TYPE_STAB, stab->Get("file")->AsString());
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

MachineConfig* MachineConfig::Create(const std::string& fileName, const std::string& homedir, QApplication *app, qarm *widget)
{
    std::auto_ptr<MachineConfig> config(new MachineConfig(fileName, app, widget));

    // The constructor initializes all the basic fields to sane
    // initial values; in addition, we enable a terminal device for
    // newly created configs.
    config->setDeviceFile(EXT_IL_INDEX(IL_TERMINAL), 0, homedir+"/term0.uarm");
    config->setDeviceEnabled(EXT_IL_INDEX(IL_TERMINAL), 0, true);

    config->Save();

    return config.release();
}

void MachineConfig::Save()
{
    scoped_ptr<JsonObject> root(new JsonObject);

    root->Set("num-processors", (int) getNumProcessors());
    root->Set("clock-rate", (int) getClockRate());
    root->Set("refresh-rate", (int) getRefreshRate());
    root->Set("refresh-on-pause", getRefreshOnPause());
    root->Set("tlb-size", (int) getTLBSize());
    root->Set("num-ram-frames", (int) getRamSize());
    root->Set("pause-on-exc", getStopOnException());
    root->Set("pause-on-tlb", getStopOnTLBChange());
    root->Set("accessible-mode", getAccessibleMode());
    root->Set("core-file", getROM(ROM_TYPE_CORE));

    root->Set("execution-rom", getROM(ROM_TYPE_BIOS));

    JsonObject* stabObject = new JsonObject;
    stabObject->Set("asid", (int) symbolTableASID);
    stabObject->Set("external-stab", externalStab);
    stabObject->Set("file", romFiles[ROM_TYPE_STAB]);
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

MachineConfig::MachineConfig(const std::string& fn, QApplication *app, qarm *widget)
    : fileName(fn),
      app(app),
      mainWidget(widget)
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

void MachineConfig::setTLBSize(Word size)
{
    tlbSize = bumpProperty(MIN_TLB_SIZE, size, MAX_TLB_SIZE);
}

void MachineConfig::setClockRate(unsigned int value)
{
    clockRate = bumpProperty(MIN_CLOCK_RATE, value, MAX_CLOCK_RATE);
}

void MachineConfig::setRefreshRate(unsigned int value)
{
    refreshRate = bumpProperty(MIN_REFRESH_RATE, value, MAX_REFRESH_RATE);
}

void MachineConfig::setRefreshOnPause(bool enabled)
{
    refreshOnPause = enabled;
}

void MachineConfig::setAccessibleMode(bool enabled){
    accessibleMode = enabled;
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

void MachineConfig::setExternalStab(bool enabled){
    externalStab = enabled;
}

void MachineConfig::setStopOnTLBChange(bool value)
{
    stopOnTLBChange = value;
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
    setClockRate(DEFAULT_CLOCK_RATE);
    setRefreshRate(DEFAULT_REFRESH_RATE);
    setRefreshOnPause(DEFAULT_REFRESH_ON_PAUSE);
    setRamSize(DEFAULT_RAM_SIZE);
    setTLBSize(DEFAULT_TLB_SIZE);
    setStopOnException(DEFAULT_STOP_ON_EXCEPTION);
    setStopOnTLBChange(DEFAULT_STOP_ON_TLB_CHANGE);

    setAccessibleMode(DEFAULT_ACCESSIBLE_MODE);

    setROM(ROM_TYPE_BIOS, "/usr/include/uarm/BIOS");

    setROM(ROM_TYPE_CORE, "kernel");
    setROM(ROM_TYPE_STAB, "");
    setSymbolTableASID(0);
    setExternalStab(false);

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
