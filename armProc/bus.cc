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

#ifndef UARM_SYSTEMBUS_CC
#define UARM_SYSTEMBUS_CC

#include "bus.h"

systemBus::systemBus(){
    if(ram == NULL)
        ram = new ramMemory();
}

systemBus::~systemBus(){
    if(ram != NULL){
        delete ram;
        ram = NULL;
    }
}

bool systemBus::fetch(Word pc, bool armMode){
    Word addr = pc - (armMode ? 8 : 4);
    ram->readW(&addr, &pipeline[PIPELINE_EXECUTE]);
    addr += 4;
    if(!armMode){
        if(!ram->readW(&addr, &pipeline[PIPELINE_DECODE]))
            return false;
        pipeline[PIPELINE_FETCH] = 0;
        return true;
    }
    ram->readW(&addr, &pipeline[PIPELINE_DECODE]);
    addr += 4;
    if(!ram->readW(&addr, &pipeline[PIPELINE_FETCH]))
        return false;
    return true;
}


Word systemBus::get_unpredictable(){
    Word ret;
    for(unsigned i = 0; i < sizeof(Word) * 8; i++)
        ret |= (rand() % 1 ? 1 : 0) << i;
    return ret;
}

bool systemBus::get_unpredictableB(){
    return rand() % 1;
}


#endif //UARM_SYSTEMBUS_CC
