/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */

#ifndef UARM_MP_CONTROLLER_H
#define UARM_MP_CONTROLLER_H

#include "armProc/types.h"

class MachineConfig;
class machine;
class systemBus;
class processor;

class MPController {
public:
    MPController(const MachineConfig* config, machine* machine);

    Word Read(Word addr, const processor* cpu) const;
    void Write(Word addr, Word data, const processor* cpu);

private:
    static const unsigned int kCpuResetDelay = 50;
    static const unsigned int kCpuHaltDelay = 50;
    static const unsigned int kPoweroffDelay = 1000;

    const MachineConfig* const config;
    machine* const mac;

    Word bootPC;
    Word bootSP;
};

#endif // UARM_MP_CONTROLLER_H
