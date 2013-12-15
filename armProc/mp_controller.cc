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

#ifndef UARM_MP_CONTROLLER_CC
#define UARM_MP_CONTROLLER_CC

#include "armProc/mp_controller.h"

#include <boost/bind.hpp>

#include "services/lang.h"
#include "armProc/machine_config.h"
#include "armProc/processor.h"
//LOOP: mp controller
#include "armProc/machine.h"
#include "armProc/bus.h"
#include "facilities/arch.h"

MPController::MPController(const MachineConfig* config, machine* mac)
    : config(config),
      mac(mac),
      bootPC(MCTL_DEFAULT_BOOT_PC),
      bootSP(MCTL_DEFAULT_BOOT_SP)
{}

Word MPController::Read(Word addr, const processor* cpu) const
{
    UNUSED_ARG(cpu);

    switch (addr) {
    case MCTL_NCPUS:
        return config->getNumProcessors();

    case MCTL_BOOT_PC:
        return bootPC;

    case MCTL_BOOT_SP:
        return bootSP;

    default:
        return 0;
    }
}

void MPController::Write(Word addr, Word data, const processor* cpu)
{
    UNUSED_ARG(cpu);

    Word cpuId;

    switch (addr) {
    case MCTL_RESET_CPU:
        cpuId = data & MCTL_RESET_CPU_CPUID_MASK;
        if (cpuId < config->getNumProcessors())
            mac->getBus()->scheduleEvent(kCpuResetDelay * config->getClockRate(),
                                             boost::bind(&Processor::Reset, mac->getProcessor(cpuId),
                                                         bootPC, bootSP));
        break;

    case MCTL_BOOT_PC:
        bootPC = data;
        break;

    case MCTL_BOOT_SP:
        bootSP = data;
        break;

    case MCTL_HALT_CPU:
        cpuId = data & MCTL_RESET_CPU_CPUID_MASK;
        if (cpuId < config->getNumProcessors())
            mac->getBus()->scheduleEvent(kCpuHaltDelay * config->getClockRate(),
                                             boost::bind(&Processor::Halt, mac->getProcessor(cpuId)));
        break;

    case MCTL_POWER:
        if (data == 0x0FF)
            mac->getBus()->scheduleEvent(kPoweroffDelay * config->getClockRate(),
                                             boost::bind(&Machine::Halt, mac));
        break;

    default:
        break;
    }
}

#endif //UARM_MP_CONTROLLER_CC
