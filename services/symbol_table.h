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

#ifndef UMPS_SYMBOL_TABLE_H
#define UMPS_SYMBOL_TABLE_H

#include <list>
#include <map>
#include <string>

#include "armProc/basic_types.h"

#include "facilities/arch.h"
#include "armProc/types.h"

// A symbol table entry. It represents a "named" address range,
// associated to the ASID of the containing symbol table.

class Symbol {
public:
    enum Type {
        TYPE_FUNCTION,
        TYPE_OBJECT
    };

    // This method builds a Symbol object, given its description as produced by
    // elf2mps utility; no format check, since it's fixed
    Symbol(const char* sName, const char* sInfo);
    ~Symbol();

    // This method defines a lexicographic order on Symbols, based on
    // start address; returns TRUE if "this" object is less than Symb,
    // and FALSE otherwise
    bool LessThan(const Symbol* other);

    // Given pos virtual address, this method returns -1 if pos is
    // out lower bound, 0 if in, 1 if out upper bound
    int Contains(Word pos) const;

    const char* getName() const { return sNamep; }

    Word getStart() const { return sStart; }
    Word getEnd() const { return sStart + sSize - WS; }

    Type getType() const { return sType; }

    SWord Offset(Word pos) const;

    // this method allows to set symbol size (backpatch for gcc bug)
    // [FIXME: What? Which GCC bug?]
    void setSize(Word size);

private:
    // Symbol name string
    char* sNamep;

    // Symbol starting address
    Word sStart;

    // Symbol size
    Word sSize;

    Type sType;
};

// SymbolTable class defines an object containing a complete symbol table. It
// is loaded from file, and is associated to a particular ASID, which comes
// from the configuration.
// For performance reasons, symbol table is split into two parts: one for
// functions and one for memory objects, since functions table will be
// accessed more often. Both SymbolTable object tables are lexicographically
// ordered for fast probing, function table is accessed for any instruction
// reference; both are inspected for mapping breakpoint/suspect/trace ranges
// to mnemonic names.

class SymbolTable {
public:
    // This method builds a symbol table from .stab file fName produced by
    // elf2mps utility
    SymbolTable(Word asid, const char * fName);
    SymbolTable(Word asid, int numF, int numO);

    ~SymbolTable();

    Word getASID() const { return asid; }

    unsigned int Size() const;

    // This method probes the table, given a complete address (asid +
    // pos): it probes both tables if fullSearch is TRUE, and only
    // function table otherwise.  It returns symbol name and offset
    // inside it, or NULL if no match is found
    const char* Probe(Word asid, Word pos, bool fullSearch, SWord* offsetp) const;
    const Symbol* Probe(Word asid, Word addr, bool fullSearch) const;

    const Symbol* Get(unsigned int index) const;
    std::list<const Symbol*> Lookup(const char* name) const;
    std::list<const Symbol*> Lookup(const char* name, Symbol::Type type) const;

    int addSymbol(const char *sName, const char *sType);
    int finalizeSymbolTable(int error);

private:
    static void sortTable(Symbol** table, size_t size);

    // This method scans the specified table looking for a Symbol range
    // containing it; returns NOTFOUND if not found, 0..size -1 if found
    // (index into table)
    int search(Symbol** table, unsigned int size, Word pos) const;

    // Symbol table ASID
    const Word asid;

    // Number of function/object symbols
    unsigned int ftSize;
    unsigned int otSize;

    // counters used in alternate construction
    unsigned int lastF, lastO;

    // Symbol tables: one for functions, other for memory object symbols
    Symbol** fTable;
    Symbol** oTable;

    typedef std::map< std::string, std::list<const Symbol*> > SymbolMap;
    SymbolMap map;
};

#endif // UMPS_SYMBOL_TABLE_H
