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

#ifndef UARM_COPROCESSOR_INTERFACE_CC
#define UARM_COPROCESSOR_INTERFACE_CC

#include "coprocessor_interface.h"

coprocessor_interface::coprocessor_interface(){
	
	coprocessors = new coprocessor*[COPROCESSORS_NUM];
	
	coprocessors[COPROCESSOR_CP15] = new cp15();
	
#ifdef COPROCESSOR_CP14
	coprocessors[COPROCESSOR_CP14] = new cp14();
#endif
#ifdef COPROCESSOR_CP7
	coprocessors[COPROCESSOR_CP7] = new cp7();
#endif
#ifdef COPROCESSOR_CP6
	coprocessors[COPROCESSOR_CP6] = new cp6();
#endif
#ifdef COPROCESSOR_CP5
	coprocessors[COPROCESSOR_CP5] = new cp5();
#endif
#ifdef COPROCESSOR_CP4
	coprocessors[COPROCESSOR_CP4] = new cp4();
#endif
}

/* hardware-encoded functions
bool coprocessor_interface::CPA(){
	bool ret = true;
	coprocessor *tmp = list;
	while(tmp != NULL){
		ret &= tmp->CPA();
		tmp = tmp->next;
	}
	return ret;
}

bool coprocessor_interface::CPB(){
	bool ret = true;
	coprocessor *tmp = list;
	while(tmp != NULL){
		ret &= tmp->CPB();
		tmp = tmp->next;
	}
	return ret;
}

void coprocessor_interface::setnCPI(bool val){
	coprocessor *tmp = list;
	while(tmp != NULL){
		tmp->setnCPI(val);
		tmp = tmp->next;
	}
}

bool coprocessor_interface::readD(int mplex, Word *data){
	switch(mplex){
	case COPROCESSOR_CP15:
		*data = coprocessors[COPROCESSOR_CP15]->getD();
		return true;
#ifdef COPROCESSOR_CP14
	case COPROCESSOR_CP14:
		*data = coprocessors[COPROCESSOR_CP14]->getD();
		return true;
#endif
#ifdef COPROCESSOR_CP7
	case COPROCESSOR_CP7:
		*data = coprocessors[COPROCESSOR_CP7]->getD();
		return true;
#endif
#ifdef COPROCESSOR_CP6
	case COPROCESSOR_CP6:
		*data = coprocessors[COPROCESSOR_CP6]->getD();
		return true;
#endif
#ifdef COPROCESSOR_CP5
	case COPROCESSOR_CP5:
		*data = coprocessors[COPROCESSOR_CP5]->getD();
		return true;
#endif
#ifdef COPROCESSOR_CP4
	case COPROCESSOR_CP4:
		*data = coprocessors[COPROCESSOR_CP4]->getD();
		return true;
#endif
	default:
		//wrong coprocessor.. undefined..
		return false;
	}
}

bool coprocessor_interface::writeD(int mplex, Word *data){
	switch(mplex){
	case COPROCESSOR_CP15:
		coprocessors[COPROCESSOR_CP15]->setD(data);
		return true;
#ifdef COPROCESSOR_CP14
	case COPROCESSOR_CP14:
		coprocessors[COPROCESSOR_CP14]->setD(data);
		return true;
#endif
#ifdef COPROCESSOR_CP7
	case COPROCESSOR_CP7:
		coprocessors[COPROCESSOR_CP7]->setD(data);
		return true;
#endif
#ifdef COPROCESSOR_CP6
	case COPROCESSOR_CP6:
		coprocessors[COPROCESSOR_CP6]->setD(data);
		return true;
#endif
#ifdef COPROCESSOR_CP5
	case COPROCESSOR_CP5:
		coprocessors[COPROCESSOR_CP5]->setD(data);
		return true;
#endif
#ifdef COPROCESSOR_CP4
	case COPROCESSOR_CP4:
		coprocessors[COPROCESSOR_CP4]->setD(data);
		return true;
#endif
	default:
		return false;
	}
}

*/

coprocessor *coprocessor_interface::getCoprocessor(Byte cpNum){
	coprocessor *ret = NULL;
	if(cpNum < COPROCESSORS_NUM)
		ret = coprocessors[cpNum];
	return ret;
}

#endif //UARM_COPROCESSOR_INTERFACE_CC
