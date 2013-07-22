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

#ifndef UARM_UTIL_CC
#define UARM_UTIL_CC

//flag operations

#include "const.h"

class util {
private:
	static util *instance;
	util() {};
public:
	~util() {};
	
	static util *getInstance() {
		if(instance == NULL)
			instance = new util();
		return instance;
	};
	
	const void setBitsReg(Word *reg, Word mask) {*reg = *reg | mask;};
	const void setBitReg(Word *reg, unsigned bit) {setBitsReg(reg, 1<<bit);};

	const void resetBitsReg(Word *reg, Word mask) {*reg = *reg & INVERT_W(mask);};
	const void resetBitReg(Word *reg, unsigned bit) {resetBitsReg(reg, 1<<bit);};

	const bool checkBits(Word *wd, Word mask) {return ((*wd & mask) == mask ? true : false);};	//returns true if all bits set in mask are
	const bool checkBits(Word wd, Word mask) {return ((wd & mask) == mask ? true : false);};	//also set in word
	const bool checkBit(Word *wd, unsigned bit) {return ((*wd & (1<<bit)) > 0 ? true : false);};
	const bool checkBit(Word wd, unsigned bit) {return ((wd & (1<<bit)) > 0 ? true : false);};
	
	const void copyBitReg(Word *reg, unsigned bit, unsigned val) {
		*reg = *reg & INVERT_W(INVERT_B(val) << bit) | (val << bit);
	};
	const void copyBitFromReg(Word *dest, unsigned bit, Word *src) {
		if(((*src>>bit)&1) != 0)	
			copyBitReg(dest, bit, 1); 
		else 
			copyBitReg(dest, bit, 0);
	};
	const void copyBitFromReg(Word *dest, unsigned dbit, Word *src, unsigned sbit) {
		if(((*src>>sbit)&1) != 0) 
			copyBitReg(dest, dbit, 1); 
		else 
			copyBitReg(dest, dbit, 0);
	};
};

#endif //UARM_UTIL_CC
