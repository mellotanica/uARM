/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uARM
 *
 * Copyright (C) 2004 Mauro Morsiani
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

#ifndef TLBENTRY_H
#define TLBENTRY_H

#include "armProc/types.h"

// Each TLBEntry object represents a single entry in the TLB contained in
// the CP0 coprocessor part of a real MIPS processor.
// Each one is a 64-bit field split in two parts (HI and LO), with special
// fields and control bits (see external documentation for more details).

class TLBEntry {
public:
    // This method builds an entry and sets its initial contents
    TLBEntry(Word entHI = 0, Word entLO = 0);

    // This method returns the HI 32-bit part of the entry
    Word getHI() const { return tlbHI; }

    // This method returns the LO 32-bit part of the entry
    Word getLO() const { return tlbLO; }

    // This method sets the entry HI part (leaving the zero-filled
    // field untouched)
    void setHI(Word entHI);

    // This method sets the entry LO part (leaving the zero-filled field
    // untouched)
    void setLO(Word entLO);

    // This method compares the entry contents with the VPN part of a
    // virtual address and returns TRUE if a match is found, FALSE
    // otherwise
    bool VPNMatch(Word vaddr);

    // This method compares the entry contents with the ASID field in a
    // CP0 special register and returns TRUE if a match is found, FALSE
    // otherwise
    bool ASIDMatch(Word cpreg);

    // the following methods return the bit value for the corresponding
    // access control bit
    bool IsG();
    bool IsV();
    bool IsD();

private:
    // contains the VPN + ASID fields, and a zero-filled field
    Word tlbHI;

    // contains the PFN field, some access control bits and a
    // zero-filled field
    Word tlbLO;
};

#endif // TLBENTRY_H
