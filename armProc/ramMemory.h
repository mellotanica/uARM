/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uARM
 *
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
 
/* 
 * This interface defines the common coprocessor - processor communication methods
 */

#ifndef UARM_MEMORY_H
#define UARM_MEMORY_H

#include "const.h"
/*#include "services/lang.h"
#include <boost/format.hpp>*/

class ramMemory{
public:
    ramMemory() {
        memVector = NULL;
        ready = false;
    }
    ~ramMemory() {
        if(memVector != NULL)
            delete [] memVector;
        memVector = NULL;
        ready = false;
    }
	
    void lockMem() { LOCK_sig = true; }
    void unlockMem() { LOCK_sig = false; }
    bool getMemLock() { return LOCK_sig; }

    void reset(Word sz){
        if(memVector == NULL || sz != ramSize){
            ready = false;
            ramSize = sz;
            if(memVector == NULL)
                delete [] memVector;
            memVector = new Byte[((DoubleWord)ramSize * 4)];
            ready = true;
        }
    }

    Word getRamSize(){
        return ramSize;
    }
	
    bool read(Word *address, Byte *dest) {return read(address, dest, ENDIANESS_BIGENDIAN);}
    bool write(Word *address, Byte data) {return write(address, data, ENDIANESS_BIGENDIAN);}
	
    bool readH(Word *address, HalfWord *dest) {return readH(address, dest, ENDIANESS_BIGENDIAN);}
    bool writeH(Word *address, HalfWord data) {return writeH(address, data, ENDIANESS_BIGENDIAN);}
	
    bool readW(Word *address, Word *dest) {return readW(address, dest, ENDIANESS_BIGENDIAN);}
    bool writeW(Word *address, Word data) {return writeW(address, data, ENDIANESS_BIGENDIAN);}
	
    bool read(Word *address, Byte *dest, bool bigEndian) {
        if(!LOCK_sig && ready){
			if(!bigEndian)
                *dest = memVector[*address];
			else
                *dest = memVector[(*address + 3 - 2 * (*address % 4))];
            return true;
		}
        return false;
    }
    bool write(Word *address, Byte data, bool bigEndian) {
        if(!LOCK_sig && ready){
			if(!bigEndian)
				memVector[*address] = data;
			else
				memVector[(*address + 3 - 2 * (*address % 4))] = data;
            return true;
		}
        return false;
    }
	
    bool readH(Word *address, HalfWord *dest, bool bigEndian) {
        if(!LOCK_sig && ready){
			HalfWord ret;
			Word addr = *address;
			Byte readb;
            if(!read(&addr, &readb, bigEndian))
                return false;
            ret = readb;
			addr++;
            if(!read(&addr, &readb, bigEndian))
                return false;
            ret |= readb << 8;
            *dest = ret;
            return true;
		}
        return false;
    }
    bool writeH(Word *address, HalfWord data, bool bigEndian) {
        if(!LOCK_sig && ready){
			Word addr = *address;
            if(!write(&addr, (Byte) data & 0xFF, bigEndian))
                return false;
			addr ++;
            if(!write(&addr, (Byte) (data >> 8) & 0xFF, bigEndian))
                return false;
            return true;
		}
        return false;
    }
	
    bool readW(Word *address, Word *dest, bool bigEndian) {
        if(!LOCK_sig && ready){
			Word addr = *address - (*address % 4);
			Word ret;
			Byte readb;
            if(!read(&addr, &readb, bigEndian))
                return false;
			ret = readb;
			addr++;
            if(!read(&addr, &readb, bigEndian))
                return false;
			ret |= readb << 8;
			addr++;
            if(!read(&addr, &readb, bigEndian))
                return false;
			ret |= readb << 16;
			addr++;
            if(!read(&addr, &readb, bigEndian))
                return false;
			ret |= readb << 24;
			if((*address % 4) != 0)
				ret = (ret >> 8*(*address%4)) | (ret << (sizeof(Word)*8 - (8*(*address%4))));
            *dest = ret;
            return true;
		}
        return false;
    }
    bool writeW(Word *address, Word data, bool bigEndian) {
        if(!LOCK_sig && ready){
			Word addr = *address;
            if(!write(&addr, (Byte) data & 0xFF, bigEndian))
                return false;
			addr ++;
            if(!write(&addr, (Byte) (data >> 8) & 0xFF, bigEndian))
                return false;
			addr ++;
            if(!write(&addr, (Byte) (data >> 16) & 0xFF, bigEndian))
                return false;
			addr ++;
            if(!write(&addr, (Byte) (data >> 24) & 0xFF, bigEndian))
                return false;
            return true;
		}
        return false;
    }
	
private:
	//scoped_array<Byte> memVector;
    Byte *memVector = NULL;
    Word ramSize = 0;
	
	bool LOCK_sig = false;
    bool ready = false;
};

#endif //UARM_MEMORY_H
