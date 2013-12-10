/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uMPS - A general purpose computer system simulator
 *
 * Copyright (C) 2004 Mauro Morsiani
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

#ifndef UMPS_TIME_STAMP_H
#define UMPS_TIME_STAMP_H

#include <string>

#include "armProc/basic_types.h"

namespace TimeStamp {

inline uint32_t getHi(uint64_t ts)
{
    return ts >> 32;
}

inline uint32_t getLo(uint64_t ts)
{
    return (uint32_t) ts;
}

inline void setHi(uint64_t& ts, uint32_t value)
{
    ts = (uint64_t) getLo(ts) | (uint64_t) value << 32;
}

inline void setLo(uint64_t& ts, uint32_t value)
{
    ts = (uint64_t) getHi(ts) | (uint64_t) value;
}

std::string toString(uint64_t ts);

} // namespace TimeStamp

#endif // UMPS_TIME_STAMP_H
