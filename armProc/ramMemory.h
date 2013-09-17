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
#include "services/lang.h"
#include <boost/format.hpp>

class ramMemory{
public:
    ramMemory() {
        if(ramSize > 0)
            init(ramSize);
        else {
            memVector = NULL;
            ready = false;
        }
    }
    ~ramMemory() {
        if(memVector != NULL){
            delete [] memVector;
            memVector = NULL;
            ready = false;
        }
    }
	
    void lockMem() { LOCK_sig = true; }
    void unlockMem() { LOCK_sig = false; }
    bool getMemLock() { return LOCK_sig; }
	
    void init(Word sz) {
        ramSize = sz;
        if(memVector == NULL){
            memVector = new Byte[((DoubleWord)ramSize*4)];
            ready = true;
        }
        //std::cout<<"RAM SIZE: "<<(ramSize * 4)<<"B";
    }
	
    Byte read(Word *address) {return read(address, false);}
    void write(Word *address, Byte data) {write(address, data, false);}
	
    HalfWord readH(Word *address) {return readH(address, false);}
    void writeH(Word *address, HalfWord data) {writeH(address, data, false);}
	
    Word readW(Word *address) {return readW(address, false);}
    void writeW(Word *address, Word data) {writeW(address, data, false);}
	
	Byte read(Word *address, bool bigEndian) {
        if(!LOCK_sig && ready){
			if(!bigEndian)
				return memVector[*address];
			else
				return memVector[(*address + 3 - 2 * (*address % 4))];
		}
		return 0;
    }
	void write(Word *address, Byte data, bool bigEndian) {
        if(!LOCK_sig && ready){
			if(!bigEndian)
				memVector[*address] = data;
			else
				memVector[(*address + 3 - 2 * (*address % 4))] = data;
		}
    }
	
	HalfWord readH(Word *address, bool bigEndian) {
        if(!LOCK_sig && ready){
			HalfWord ret;
			Word addr = *address;
			Byte readb;
			readb = read(&addr, bigEndian);
			ret = readb;
			addr++;
			readb = read(&addr, bigEndian);
			ret |= readb << 8;
			return ret;
		}
		return 0;
    }
	void writeH(Word *address, HalfWord data, bool bigEndian) {
        if(!LOCK_sig && ready){
			Word addr = *address;
			write(&addr, (Byte) data & 0xFF, bigEndian);
			addr ++;
			write(&addr, (Byte) (data >> 8) & 0xFF, bigEndian);
		}
    }
	
	Word readW(Word *address, bool bigEndian) {
        if(!LOCK_sig && ready){
			Word addr = *address - (*address % 4);
			Word ret;
			Byte readb;
			readb = read(&addr, bigEndian);
			ret = readb;
			addr++;
			readb = read(&addr, bigEndian);
			ret |= readb << 8;
			addr++;
			readb = read(&addr, bigEndian);
			ret |= readb << 16;
			addr++;
			readb = read(&addr, bigEndian);
			ret |= readb << 24;
			if((*address % 4) != 0)
				ret = (ret >> 8*(*address%4)) | (ret << (sizeof(Word)*8 - (8*(*address%4))));
			return ret;
		}
		return 0;
    }
	void writeW(Word *address, Word data, bool bigEndian) {
        if(!LOCK_sig && ready){
			Word addr = *address;
			write(&addr, (Byte) data & 0xFF, bigEndian);
			addr ++;
			write(&addr, (Byte) (data >> 8) & 0xFF, bigEndian);
			addr ++;
			write(&addr, (Byte) (data >> 16) & 0xFF, bigEndian);
			addr ++;
			write(&addr, (Byte) (data >> 24) & 0xFF, bigEndian);
		}
    }
	
private:
	//scoped_array<Byte> memVector;
	Byte *memVector;
    Word ramSize = 0;
	
	bool LOCK_sig = false;
    bool ready = false;
};

#endif //UARM_MEMORY_H
