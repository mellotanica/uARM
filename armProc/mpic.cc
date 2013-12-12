/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uMPS - A general purpose computer system simulator
 *
 * Copyright (C) 2011 Tomislav Jonjic
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

#ifndef UARM_MPIC_CC
#define UARM_MPIC_CC

#include "armProc/mpic.h"

#include <boost/bind.hpp>

#include "services/debug.h"
#include "armProc/machine_config.h"
#include "armProc/bus.h"
#include "armProc/processor.h"

InterruptController::InterruptController(const MachineConfig* config, systemBus* bus)
    : config(config),
      bus(bus),
      arbiter(0),
      cpuData(config->getNumProcessors())
{}

void InterruptController::StartIRQ(unsigned int il, unsigned int devNo)
{
    il -= kBaseIL;
    assert(il >= kSharedILBase || !devNo);

    // Obtain source routing info
    Source& source = sources[il][devNo];
    Word target = kInvalidCpuId;
    if (source.route.policy == IRT_POLICY_FIXED) {
        if (source.route.destination < cpuData.size())
            target = source.route.destination;
    } else {
        for (size_t i = 0; i < cpuData.size(); i++) {
            Word id = (arbiter + i) % cpuData.size();
            if (source.route.destination & (1U << id)) {
                if (target == kInvalidCpuId || cpuData[id].taskPriority > cpuData[target].taskPriority)
                    target = id;
            }
        }
        if (target != kInvalidCpuId)
            arbiter = (target + 1) % cpuData.size();
    }

    // No further work to do if no valid target cpu was found;
    // interrupt is lost forever.
    if (target == kInvalidCpuId)
        return;

    source.lastTarget = target;
    cpuData[target].ipMask |= 1U << (kBaseIL + il);

    // For shared int. lines, also set the appropriate bit in the
    // interrupting devices bitmap
    if (il >= kSharedILBase)
        cpuData[target].idb[il - kSharedILBase] |= 1U << devNo;

    bus->AssertIRQ(kBaseIL + il, target);
}

void InterruptController::EndIRQ(unsigned int il, unsigned int devNo)
{
    il -= kBaseIL;
    assert(il >= kSharedILBase || !devNo);

    // This might be a "spurious" acknowledge message in case the
    // interrupt wasn't delivered to any core.
    Word target = sources[il][devNo].lastTarget;
    if (target == kInvalidCpuId)
        return;

    // Deassert IP signals and IDB bits
    if (il >= kSharedILBase) {
        cpuData[target].idb[il - kSharedILBase] &= ~(1U << devNo);
        if (!cpuData[target].idb[il - kSharedILBase]) {
            cpuData[target].ipMask &= ~(1U << (kBaseIL + il));
            bus->DeassertIRQ(kBaseIL + il, target);
        }
    } else {
        cpuData[target].ipMask &= ~(1U << (kBaseIL + il));
        bus->DeassertIRQ(kBaseIL + il, target);
    }

    sources[il][devNo].lastTarget = kInvalidCpuId;
}

Word InterruptController::Read(Word addr, const processor* cpu) const
{
    if (CDEV_BITMAP_BASE <= addr && addr < CDEV_BITMAP_END)
        return cpuData[cpu->Id()].idb[(addr - CDEV_BITMAP_BASE) >> 2];

    if (IRT_BASE <= addr && addr < IRT_END) {
        unsigned int offset = (addr - IRT_BASE) >> 2;
        unsigned int il = offset / N_DEV_PER_IL;
        unsigned int slot = offset % N_DEV_PER_IL;
        const Source& s = sources[il][slot];
        return s.route.destination | (s.route.policy << IRT_ENTRY_POLICY_BIT);
    }

    if (CPUCTL_BASE <= addr && addr < CPUCTL_END) {
        const CpuData& cd = cpuData[cpu->Id()];

        switch (addr) {
        case CPUCTL_INBOX:
            if (!cd.ipiInbox.empty()) {
                IpiMessage ipi = cd.ipiInbox.front();
                return ipi.msg | (ipi.origin << CPUCTL_INBOX_ORIGIN_BIT);
            } else {
                return 0;
            }

        case CPUCTL_TPR:
            return cd.taskPriority;

        case CPUCTL_BIOS_RES_0:
            return cd.biosReserved[0];

        case CPUCTL_BIOS_RES_1:
            return cd.biosReserved[1];

        default:
            return 0;
        }
    }

    AssertNotReached();

    return 0;
}

void InterruptController::Write(Word addr, Word data, const processor* cpu)
{
    if (IRT_BASE <= addr && addr < IRT_END) {
        unsigned int offset = (addr - IRT_BASE) >> 2;
        unsigned int il = offset / N_DEV_PER_IL;
        unsigned int slot = offset % N_DEV_PER_IL;
        Source& s = sources[il][slot];
        s.route.destination = IRT_ENTRY_GET_DEST(data);
        s.route.policy = IRT_ENTRY_GET_POLICY(data);
    } else if (CPUCTL_BASE <= addr && addr < CPUCTL_END) {
        CpuData& cd = cpuData[cpu->Id()];

        switch (addr) {
        case CPUCTL_INBOX:
            if (!cd.ipiInbox.empty()) {
                cd.ipiInbox.pop_front();
                if (cd.ipiInbox.empty()) {
                    cd.ipMask &= ~(1U << IL_IPI);
                    bus->DeassertIRQ(IL_IPI, cpu->Id());
                }
            }
            break;

        case CPUCTL_OUTBOX:
            bus->scheduleEvent(kIpiLatency * config->getClockRate(),
                               boost::bind(&InterruptController::deliverIPI, this, cpu->Id(), data));
            break;

        case CPUCTL_TPR:
            cd.taskPriority = data & CPUCTL_TPR_PRIORITY_MASK;
            break;

        case CPUCTL_BIOS_RES_0:
            cd.biosReserved[0] = data;
            break;

        case CPUCTL_BIOS_RES_1:
            cd.biosReserved[1] = data;
            break;

        default:
            break;
        }
    }
}

void InterruptController::deliverIPI(unsigned int origin, Word outbox)
{
    Word recipients = CPUCTL_OUTBOX_GET_RECIP(outbox);

    for (unsigned int i = 0; i < config->getNumProcessors(); i++) {
        if (recipients & (1U << i)) {
            bool hasSlot = true;
            foreach (const IpiMessage& ipi, cpuData[i].ipiInbox) {
                if (ipi.origin == origin) {
                    hasSlot = false;
                    break;
                }
            }
            if (hasSlot) {
                IpiMessage ipi;
                ipi.origin = origin;
                ipi.msg = CPUCTL_OUTBOX_GET_MSG(outbox);
                cpuData[i].ipiInbox.push_back(ipi);
                cpuData[i].ipMask |= 1U << IL_IPI;
                bus->AssertIRQ(IL_IPI, i);
            }
        }
    }
}

#endif //UARM_MPIC_CC
