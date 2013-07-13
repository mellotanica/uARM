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
 * Interface common to pocessing units (main processor and coprocessors)
 * Provides the common execution structure
 */

#ifndef UARM_PU_H
#define UARM_PU_H

#include "const.h"
#include "bus.h"

class pu{
public:
	pu() {
		if(bus == NULL)
			bus = new systemBus();
		for(int i = 0; i < PIPELINE_STAGES; i++){
			pipeline[i] = 0;
		}
	};
	~pu() {};
	
	pu* next;
	
	Word *getPipeline(unsigned int i) {return &pipeline[i];};
	
	/* processor could abort the execution cycle of coprocessors in case of interrupts or traps
	 * also, the first two cycles only do fetches to load the pipeline
	 */
	virtual void cycle() {
		fetch();
		decode();
		setOP("Unknown");
		execute();
	};
	
	Word OPcode;
	bool isOPcodeARM = true;
	string mnemonicOPcode;
	
protected:
	static systemBus *bus;
	Word pipeline[PIPELINE_STAGES];
	
	virtual void setOP(string mnemonic){
		isOPcodeARM = true;
		OPcode = pipeline[PIPELINE_EXECUTE];
		mnemonicOPcode = mnemonic;
	}
	
	virtual void fetch() {pipeline[PIPELINE_EXECUTE] = pipeline[PIPELINE_DECODE]; pipeline[PIPELINE_DECODE] = pipeline[PIPELINE_FETCH]; pipeline[PIPELINE_FETCH] = bus->currentFetch;};
	virtual void decode() {};
	virtual void execute() = 0;
	
};

class coprocessor : public pu{
public:
	coprocessor() : pu() {};
	~coprocessor() {};
	
	coprocessor *next;
	
	void setnCPI(bool val) {nCPI_line = val;};
	bool CPA() {return CPA_line;};
	bool CPB() {return CPB_line;};
	Word getD()	{return D;};
	void setD(Word *data) {D = *data;};
	
protected:
	Word D;
	bool CPA_line, CPB_line, nCPI_line;
};

#endif //UARM_PU_H
