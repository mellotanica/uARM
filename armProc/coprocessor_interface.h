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
 * This interface simulates the physical communications between main processor and coprocessors
 */
 
#ifndef UARM_COPROCESSOR_INTERFACE_H
#define UARM_COPROCESSOR_INTERFACE_H

#include "armProc/const.h"
#include "armProc/cp15.h"
#include "armProc/pu.h"

class coprocessor_interface{
public:
	coprocessor_interface();
    ~coprocessor_interface();

    void reset();
	
	/* hardware-encoded functions
	void setnCPI(bool val);
	
	bool CPA();
	bool CPB();
	
	bool readD(int mplex, Word *data);
	bool writeD(int mplex, Word *data);
	*/
	
	coprocessor *getCoprocessor(Byte cpNum);
	
private:
	coprocessor **coprocessors;
};

#endif //UARM_COPROCESSOR_INTERFACE_H
