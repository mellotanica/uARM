/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uMPS - A general purpose computer system simulator
 *
 * Copyright (C) 2004 Mauro Morsiani
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

#include "services/symbol_table.h"

#include <string.h>
#include <stdio.h>

#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <functional>
#include <boost/bind.hpp>

#include "armProc/const.h"
#include "armProc/blockdev_params.h"
#include "services/disass.h"
#include "services/utility.h"
#include "services/error.h"
#include "services/debug.h"

static const unsigned int STRBUFSIZE = 64;

#define NOTFOUND -1

// This method builds a Symbol object, given its description as produced by
// elf2mps utility; no format check, since it's fixed
Symbol::Symbol(const char* sName, const char* sInfo)
{
    sNamep = new char [strlen(sName) + 1];
    strcpy(sNamep, sName);

    if (sInfo[0] == 'F')
        sType = TYPE_FUNCTION;
    else
        sType = TYPE_OBJECT;

    // get start and size
    const char * nump;
    nump = strchr(sInfo, ':') + 1;
    StrToWord(nump, &sStart);
    nump = strchr(nump, ':') + 1;
    StrToWord(nump, &sSize);

    // backpatch for gcc 3.3.3 "size == 0" bug
    // and for very small objects
    if (sSize < WORDLEN)
        sSize = WORDLEN;
}

// This method deletes a Symbol object and contained items
Symbol::~Symbol()
{
    delete[] sNamep;
}

// This method defines a lexicographic order on Symbols, based on start
// address; returns TRUE if "this" object is less than Symb, and FALSE
// otherwise
bool Symbol::LessThan(const Symbol* other)
{
    return sStart < other->sStart;
}

// Given pos virtual address, this method returns -1 if pos is
// out lower bound, 0 if in, 1 if out upper bound
int Symbol::Contains(Word pos) const
{
    if (sStart > pos)
        return -1;
    else if (pos >= (sStart + sSize))
        return 1;
    else
        return 0;
}

SWord Symbol::Offset(Word pos) const
{
    return (SWord) (pos - sStart);
}

void Symbol::setSize(Word size)
{
    sSize = size;
}

// This method builds a symbol table from .stab file fName produced by
// elf2mps utility
SymbolTable::SymbolTable(Word asid, const char* fName)
    : asid(asid)
{
    assert(fName != NULL);

    FILE* file;

    unsigned int numF, numO, i;
    char sName[STRBUFSIZE];
    char sType[STRBUFSIZE];

    fTable = NULL;
    ftSize = 0;
    oTable = NULL;
    otSize = 0;

    if ((file = fopen(fName, "r")) == NULL) {
        throw FileError(fName);
    } else {
        Word tag;
        if (fread((void *) &tag, WS, 1, file) != 1) {
            fclose(file);
            throw FileError(fName);
        }
        if (tag != STABFILEID || fscanf(file, "%X %X ", &numF, &numO) != 2) {
            fclose(file);
            throw InvalidFileFormatError(fName, "Invalid symbol table file format");
        }
    }

    // inits symbol table structures
    ftSize = numF;
    otSize = numO;
    if (ftSize > 0) {
        fTable = new Symbol* [ftSize];
        for (i = 0; i < ftSize; i++)
            fTable[i] = NULL;
    }
    if (otSize > 0)  {
        oTable = new Symbol* [otSize];
        for (i = 0; i < otSize; i++)
            oTable[i] = NULL; 
    }

    // scans symbol table file and builds objects
    bool error = false;
    for (i = 0, numF = 0, numO = 0; i < (ftSize + otSize) && !error; i++) {
        if (fscanf(file, "%s :%s ", sName, sType) != 2) {
            error = true;
        } else if (sType[0] == 'F') {
            fTable[numF] = new Symbol(sName, sType);
            numF++;
        } else {
            // sType[0] is 'O'
            oTable[numO] = new Symbol(sName, sType);
            numO++;
        }
    }

    if (error) {
        // Clean up first
        if (ftSize > 0) {
            for (i = 0; i < ftSize; i++)
                if (fTable[i] != NULL)
                    delete fTable[i];
            delete fTable;
        }
        if (otSize > 0) {
            for (i = 0; i < otSize; i++)
                if (oTable[i] != NULL)
                    delete oTable[i];
            delete oTable;
        }
        throw InvalidFileFormatError(fName, "Invalid symbol table file format");
    }

    // sort tables
    if (ftSize > 1) {
        sortTable(fTable, ftSize);

        // backpatch FUN part of symbol table for gcc bug
        for (i = 0; i < ftSize - 1; i++)
            if (fTable[i]->getEnd() == fTable[i]->getStart())
                // backpatch needed
                fTable[i]->setSize(fTable[i+1]->getStart() - fTable[i]->getStart());
    }

    if (otSize > 1)
        sortTable(oTable, otSize);

    for (unsigned int i = 0; i < Size(); i++) {
        const Symbol* s = Get(i);
        map[s->getName()].push_back(s);
    }

    fclose(file);
}

SymbolTable::~SymbolTable()
{
    unsigned int i;
	
    if (fTable != NULL) {
        for (i = 0; i < ftSize; i++)
            delete fTable[i];
        delete[] fTable;
    }

    if (oTable != NULL) {
        for (i = 0; i < otSize; i++)
            delete oTable[i];
        delete[] oTable;
    }
}

// This method probes the table, given a complete address (asid +
// pos): it probes both tables if fullSearch is TRUE, and only
// function table otherwise.  It returns symbol name and offset of pos
// inside it, or NULL if no match is found
const char* SymbolTable::Probe(Word asid, Word pos, bool fullSearch, SWord* offsetp) const
{
    if (asid != this->asid)
        return NULL;

    int idx;
    if ((idx = search(fTable, ftSize, pos)) != NOTFOUND) {
        *offsetp = fTable[idx]->Offset(pos);
        return fTable[idx]->getName();
    } else if (fullSearch && (idx = search(oTable, otSize, pos)) != NOTFOUND) {
        *offsetp = oTable[idx]->Offset(pos);
        return oTable[idx]->getName();
    } else
        return NULL;
}

const Symbol* SymbolTable::Probe(Word asid, Word addr, bool fullSearch) const
{
    if (asid != this->asid)
        return NULL;

    int i;
    if ((i = search(fTable, ftSize, addr)) != NOTFOUND)
        return fTable[i];
    else if (fullSearch && (i = search(oTable, otSize, addr)) != NOTFOUND)
        return oTable[i];
    else
        return NULL;
}

// This method returns the total number of symbols
unsigned int SymbolTable::Size() const
{
    return otSize + ftSize;
}

const Symbol* SymbolTable::Get(unsigned int index) const
{
    if (index < ftSize)
        return fTable[index];
    else
        return oTable[index - ftSize];
}

std::list<const Symbol*> SymbolTable::Lookup(const char* name) const
{
    SymbolMap::const_iterator it = map.find(name);
    if (it != map.end())
        return it->second;
    else
        return std::list<const Symbol*>();
}

std::list<const Symbol*> SymbolTable::Lookup(const char* name, Symbol::Type type) const
{
    std::list<const Symbol*> symbols = Lookup(name);
    std::list<const Symbol*> subset;
    std::remove_copy_if(symbols.begin(), symbols.end(), std::back_inserter(subset),
                        boost::bind(std::not_equal_to<Symbol::Type>(),
                                    boost::bind(&Symbol::getType, _1),
                                    type));
    return subset;
}

// This method scans the specified table looking for a Symbol range
// containing it; returns NOTFOUND if not found, 0..size -1 if found (index
// into table)
// Almost classical binary search
int SymbolTable::search(Symbol** table, unsigned int size, Word pos) const
{
    if (table == NULL)
        return NOTFOUND;

    int a = 0;
    int b;
    int c = size - 1;
    int res;
    bool found = false;

    do {
        b = (a + c) / 2;
        if ((res = (table[b])->Contains(pos)) == 0)
            found = true;
        else
            // res is > 0 if pos is (possibly) in an area with
            // start address greater than current, < 0 otherwise
            if (res > 0)
                a = b + 1;
            else
                c = b - 1;
    } while (!found && a <= c);

    return found ? b : NOTFOUND;
}
		
void SymbolTable::sortTable(Symbol** table, size_t size)
{
    struct Compare {
        static bool InOrder(Symbol* s1, Symbol* s2) {
            return s1->LessThan(s2);
        }
    };
    std::sort(table, table + size, Compare::InOrder);
}
