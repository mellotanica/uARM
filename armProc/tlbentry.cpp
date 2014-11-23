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

#include "armProc/tlbentry.h"
#include "armProc/const.h"
#include "services/utility.h"

// This method builds an entry and sets its initial contents
TLBEntry::TLBEntry(Word entHI, Word entLO)
{
    tlbHI = entHI;
    tlbLO = entLO;
}

// This method sets the entry HI part (leaving the zero-filled field untouched)
void TLBEntry::setHI(Word entHI)
{
    tlbHI = (entHI & (VPNMASK | ASIDMASK));
}

// This method sets the entry LO part (leaving the zero-filled field untouched)
void TLBEntry::setLO(Word entLO)
{
    tlbLO = (entLO & ENTRYLOMASK);
}

// This method compares the entry contents with the VPN part of a virtual
// address and returns TRUE if a match is found, FALSE otherwise
bool TLBEntry::VPNMatch(Word vaddr)
{
    return VPN(tlbHI) == VPN(vaddr);
}

// This method compares the entry contents with the ASID field in a CP0
// special register and returns TRUE if a match is found, FALSE otherwise
bool TLBEntry::ASIDMatch(Word cpreg)
{
    return ASID(tlbHI) == ASID(cpreg);
}

// This method returns the value of G (Global) access control bit in an
// entry LO part
bool TLBEntry::IsG()
{
    return BitVal(tlbLO, GBITPOS);
}

// This method returns the value of V (Valid) access control bit in an
// entry LO part
bool TLBEntry::IsV()
{
    return BitVal(tlbLO, VBITPOS);
}

// This method returns the value of D (Dirty) access control bit in an
// entry LO part
bool TLBEntry::IsD()
{
    return BitVal(tlbLO, DBITPOS);
}
