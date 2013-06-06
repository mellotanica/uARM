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

class ramMemory{
public:
	ramMemory() {
		uint64_t size = (uint64_t)MEM_SIZE_B;
		memVector = new Byte [size]; 
	};
	~ramMemory() {delete [] memVector;};
	
	Byte read(Word *address) {return read(address, false);};
	void write(Word *address, Byte data) {write(address, data, false);};
	
	Word readW(Word *address) {return readW(address, false);};
	
	void writeW(Word *address, Word data) {writeW(address, data, false);};
	
	Byte read(Word *address, bool bigEndian) {
		if(!bigEndian)
			return memVector[*address];
		else
			return memVector[(*address + 3 - 2 * (*address % 4))];
	};
	void write(Word *address, Byte data, bool bigEndian) {
		if(!bigEndian)
			memVector[*address] = data;
		else
			memVector[(*address + 3 - 2 * (*address % 4))] = data;
	};
	
	Word readW(Word *address, bool bigEndian) {
		Word addr = *address - (*address % 4);
		Word ret;
		Byte read;
		if(!bigEndian){
			read = memVector[addr++];
			ret = read;
			read = memVector[addr++];
			ret |= read << 8;
			read = memVector[addr++];
			ret |= read << 16;
			read = memVector[addr];
			ret |= read << 24;
			if((*address % 4) != 0)
				ret = (ret >> 8*(*address%4)) | (ret << (sizeof(Word)*8 - (8*(*address%4))));
			return ret;
		}
		else {
			addr += 4;
			read = memVector[addr--];
			ret = read;
			read = memVector[addr--];
			ret |= read << 8;
			read = memVector[addr--];
			ret |= read << 16;
			read = memVector[addr];
			ret |= read << 24;
			if((*address % 4) != 0)
				ret = (ret << 8*(*address%4)) | (ret >> (sizeof(Word)*8 - (8*(*address%4))));
			return ret;
		}
	};
	
	void writeW(Word *address, Word data, bool bigEndian) {
		Word addr = *address;
		if(!bigEndian){
			memVector[addr++] = (Byte) data & 0xFF ;
			memVector[addr++] = (Byte) (data >> 8) & 0xFF;
			memVector[addr++] = (Byte) (data >> 16) & 0xFF;
			memVector[addr] = (Byte) (data >> 24) & 0xFF;
		}
		else {
			memVector[addr++] = (Byte) (data >> 24) & 0xFF ;
			memVector[addr++] = (Byte) (data >> 16) & 0xFF;
			memVector[addr++] = (Byte) (data >> 8) & 0xFF;
			memVector[addr] = (Byte) data & 0xFF;
		}
	};
	
private:
	Byte *memVector;
};

#endif //UARM_MEMORY_H
