/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uARM - A general purpose computer system simulator
 *
 * Copyright (C) 2004 Mauro Morsiani
 * Copyright (C) 2011 Tomislav Jonjic
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
 * This is a stand-alone program which converts an ELF object or executable
 * file into one of MPS project file formats: .core, .rom or .aout.
 * See external documentation for format details.
 */

//#include <config.h>  //to be added when configl will work..

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <cstdlib>
#include <iterator>
#include <cassert>
#include <stdint.h>
#include <algorithm>
#include <cstring>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>

#include "services/util.h"
#include "services/debug.h"
#include "services/elf2arm.h"
#include "armProc/blockdev_params.h"
#include "facilities/arch.h"

#ifndef MKDEV_BUILD
#include "services/symbol_table.h"
#endif

/*
 * Functions throughtout this module close over this global!
 */


static const size_t kBlockSize = 4096;

#define forEachSection(sd, elf) \
    for (sd = elf_nextscn(elf, NULL); sd != NULL; sd = elf_nextscn(elf, sd))

#define forEachSectionData(sd, data) \
    for (data = elf_getdata(sd, NULL); data != NULL; data = elf_getdata(sd, data))

#define elfError(emsg) ({*error = (char *) emsg; return 1;})

#ifndef MKDEV_BUILD
struct elfSymbol {
    elfSymbol(const std::string& name, Elf32_Sym* details)
        : name(name), details(details) {}

    std::string name;
    Elf32_Sym* details;
};

/*
 * Iterator class that factors out the logic behind the iteration
 * through an ELF symbol table.
 */
class elfSymbolTableIterator : public std::iterator<std::input_iterator_tag, elfSymbol>
{
public:
    explicit elfSymbolTableIterator(Elf* elf);
    elfSymbolTableIterator();

    elfSymbolTableIterator& operator++();
    elfSymbolTableIterator operator++(int);

    elfSymbol operator*() const;

    bool operator==(const elfSymbolTableIterator& other) { return current == other.current; }
    bool operator!=(const elfSymbolTableIterator& other) { return current != other.current; }

private:
    Elf* elf;
    Elf_Scn* const stSec;
    Elf32_Shdr* const stSecHeader;
    Elf_Data* data;
    Elf32_Sym* current;
};
#endif

uint32_t toTargetEndian(uint32_t x);

const char* programName;

// Input file name
const char* fileName = NULL;

// Be verbose
// static bool verbose = false;

#ifndef MKDEV_BUILD
static Elf_Scn* getSectionByType(Elf32_Word type, Elf *elf)
{
    Elf_Scn* sd;
    forEachSection(sd, elf) {
        Elf32_Shdr* sh = elf32_getshdr(sd);
        if (sh == NULL)
            return NULL;
        if (sh->sh_type == type)
            return sd;
    }
    return NULL;
}
#endif

int initf(const char * fileName, e2adata* data, char **error)
{
    if (elf_version(EV_CURRENT) == EV_NONE)
        elfError("ELF library out of date");

    if (strcmp(fileName, "") == 0) {
        elfError("File not found");
    }

    data->file = open(fileName, O_RDONLY);
    if (data->file == -1)
        elfError("Cannot access file");

    data->elf = elf_begin(data->file, ELF_C_READ, NULL);
    if (data->elf == NULL || elf_kind(data->elf) != ELF_K_ELF)
        elfError("Not a regular ELF file");

    // Check ELF file version and arch
    data->elfHeader = elf32_getehdr(data->elf);
    if (data->elfHeader == NULL)
        elfError("Not a regular ELF file");
    if (data->elfHeader->e_version != EV_CURRENT)
        elfError("Unsupported ELF file version");
    if (data->elfHeader->e_machine != EM_ARM)
        elfError("Invalid ELF architecture (ARM expected)");
    return 0;
}

inline uint32_t toTargetEndian(uint32_t x, Elf32_Ehdr *elfHeader)
{
#ifdef WORDS_BIGENDIAN
    return (elfHeader->e_ident[EI_DATA] == ELFDATA2LSB) ? SwapEndian32(x) : x;
#else
    return (elfHeader->e_ident[EI_DATA] == ELFDATA2LSB) ? x : SwapEndian32(x);
#endif
}

#ifndef MKDEV_BUILD
elfSymbolTableIterator::elfSymbolTableIterator(Elf* elf)
    : elf(elf),
      stSec(getSectionByType(SHT_SYMTAB, elf)),
      stSecHeader(elf32_getshdr(stSec))
{
    current = NULL;

    data = NULL;
    while ((data = elf_getdata(stSec, data)) != NULL)
        if (data->d_type == ELF_T_SYM && data->d_size > 0)
            break;
    if (data != NULL)
        current = (Elf32_Sym*) data->d_buf;
}

elfSymbolTableIterator::elfSymbolTableIterator()
    : stSec(NULL), stSecHeader(NULL), current(NULL)
{}

elfSymbol elfSymbolTableIterator::operator*() const
{
    assert(current != NULL);

    const char* name = elf_strptr(elf, stSecHeader->sh_link, current->st_name);
    assert(name != NULL);

    return elfSymbol(name, current);
}

elfSymbolTableIterator& elfSymbolTableIterator::operator++()
{
    if (current == NULL)
        return *this;

    ++current;
    if (current >= (Elf32_Sym*) data->d_buf + data->d_size / sizeof(Elf32_Sym)) {
        while ((data = elf_getdata(stSec, data)) != NULL)
            if (data->d_type == ELF_T_SYM && data->d_size > 0)
                break;
        if (data == NULL)
            current = NULL;
        else
            current = (Elf32_Sym*) data->d_buf;
    }

    return *this;
}

elfSymbolTableIterator elfSymbolTableIterator::operator++(int)
{
    elfSymbolTableIterator result = *this;
    ++(*this);
    return result;
}
#endif

static int readCore(e2adata *edata, char **error, coreElf *newelf, uint8_t **dBuf, uint8_t **tBuf, Word *dSize, Word *tSize){
    // Check ELF object type
    if (edata->elfHeader->e_type != ET_EXEC)
        elfError("ELF object file is not executable");

    uint32_t *header = newelf->header;
    std::fill_n(header, N_AOUT_HDR_ENT, 0);

    // Set program entry
    header[AOUT_HE_ENTRY] = edata->elfHeader->e_entry;

    // Obtain the program header table
    Elf32_Phdr* pht = elf32_getphdr(edata->elf);
    if (pht == NULL)
        elfError("Not a regular ELF file");
    size_t phtSize;
    if (elf_getphdrnum(edata->elf, &phtSize))
        elfError("Not a regular ELF file");

    // Obtain segment info
    bool foundDataSeg = false;
    uint8_t* dataBuf = NULL;
    bool foundTextSeg = false;
    uint8_t* textBuf = NULL;

#ifndef MKDEV_BUILD
    header[AOUT_HE_TAG] = COREFILEID;

    //prepare data and text buffers
    for (size_t i = 0; i < phtSize; i++) {
        if (pht[i].p_type != PT_LOAD)
            continue;
        if (pht[i].p_flags == (PF_R | PF_W)) {// data segmet
            if (foundDataSeg)
                elfError("Redundant .data program header table entry");
            foundDataSeg = true;
            header[AOUT_HE_DATA_FILESZ] = pht[i].p_filesz;
            header[AOUT_HE_DATA_VADDR] = pht[i].p_vaddr;
            header[AOUT_HE_DATA_MEMSZ] = (((uint32_t)pht[i].p_memsz) / kBlockSize) * kBlockSize; //rounds memory size to 4k mem blocks sixe (actual memory to be allocated)
            if (header[AOUT_HE_DATA_MEMSZ] < header[AOUT_HE_DATA_FILESZ])
                header[AOUT_HE_DATA_MEMSZ] += kBlockSize;
            if (header[AOUT_HE_DATA_MEMSZ] > 0) {
                dataBuf = new uint8_t[header[AOUT_HE_DATA_MEMSZ]];
                *dSize = header[AOUT_HE_DATA_MEMSZ];
                std::fill_n(dataBuf, header[AOUT_HE_DATA_MEMSZ], 0);
            } else {
                dataBuf = NULL;
            }
        } else if (pht[i].p_flags == (PF_R | PF_X)) {// text segment
            if (foundTextSeg)
                elfError("Redundant .text program header table entry");
            if (pht[i].p_memsz == 0)
                elfError("Empty .text segment");
            foundTextSeg = true;
            header[AOUT_HE_TEXT_FILESZ] = pht[i].p_filesz;
            header[AOUT_HE_TEXT_VADDR] = pht[i].p_vaddr;
            header[AOUT_HE_TEXT_MEMSZ] = (((uint32_t)pht[i].p_memsz) / kBlockSize) * kBlockSize;
            if (header[AOUT_HE_TEXT_MEMSZ] < header[AOUT_HE_TEXT_FILESZ])
                header[AOUT_HE_TEXT_MEMSZ] += kBlockSize;
            textBuf = new uint8_t[header[AOUT_HE_TEXT_MEMSZ]];
            *tSize = header[AOUT_HE_TEXT_MEMSZ];
            std::fill_n(textBuf, header[AOUT_HE_TEXT_MEMSZ], 0);
        } else {
            fprintf(stderr, "Warning: unknown program header table entry %u\n", (unsigned int) i);
        }
    }

    if (!foundTextSeg)
        elfError("Missing .text program header");

    if (!foundDataSeg) {
        header[AOUT_HE_DATA_MEMSZ] = header[AOUT_HE_DATA_FILESZ] = 0;
        header[AOUT_HE_DATA_VADDR] = header[AOUT_HE_TEXT_VADDR] + header[AOUT_HE_TEXT_MEMSZ];
        dataBuf = NULL;
    }

    header[AOUT_HE_TEXT_OFFSET] = 0;
    header[AOUT_HE_DATA_OFFSET] = header[AOUT_HE_TEXT_MEMSZ];
#else
    header[AOUT_HE_TAG] = AOUTFILEID;

    //prepare data and text buffers
    for (size_t i = 0; i < phtSize; i++) {
        if (pht[i].p_type != PT_LOAD)
            continue;
        if (pht[i].p_flags == (PF_R | PF_W)) {// data segmet
            if (foundDataSeg)
                elfError("Redundant .data program header table entry");
            foundDataSeg = true;
            header[AOUT_HE_DATA_MEMSZ] = pht[i].p_memsz;
            header[AOUT_HE_DATA_VADDR] = pht[i].p_vaddr;
            header[AOUT_HE_DATA_FILESZ] = (pht[i].p_filesz / kBlockSize) * kBlockSize; //rounds memory size to 4k mem blocks sixe (actual memory to be allocated)
            if (header[AOUT_HE_DATA_FILESZ] < pht[i].p_filesz)
                header[AOUT_HE_DATA_FILESZ] += kBlockSize;
            if (header[AOUT_HE_DATA_FILESZ] > 0) {
                dataBuf = new uint8_t[header[AOUT_HE_DATA_FILESZ]];
                *dSize = header[AOUT_HE_DATA_FILESZ];
                std::fill_n(dataBuf, header[AOUT_HE_DATA_FILESZ], 0);
            } else {
                dataBuf = NULL;
            }
        } else if (pht[i].p_flags == (PF_R | PF_X)) {// text segment
            if (foundTextSeg)
                elfError("Redundant .text program header table entry");
            if (pht[i].p_memsz == 0)
                elfError("Empty .text segment");
            foundTextSeg = true;
            header[AOUT_HE_TEXT_MEMSZ] = pht[i].p_memsz;
            header[AOUT_HE_TEXT_VADDR] = pht[i].p_vaddr;
            header[AOUT_HE_TEXT_FILESZ] = ((AOUT_HE_TEXT_MEMSZ) / kBlockSize) * kBlockSize;
            if (header[AOUT_HE_TEXT_FILESZ] < header[AOUT_HE_TEXT_MEMSZ])
                header[AOUT_HE_TEXT_FILESZ] += kBlockSize;
            textBuf = new uint8_t[header[AOUT_HE_TEXT_FILESZ]];
            *tSize = header[AOUT_HE_TEXT_FILESZ];
            std::fill_n(textBuf, header[AOUT_HE_TEXT_FILESZ], 0);
        } else {
            fprintf(stderr, "Warning: unknown program header table entry %u\n", (unsigned int) i);
        }
    }

    if (!foundTextSeg)
        elfError("Missing .text program header");

    if (!foundDataSeg) {
        header[AOUT_HE_DATA_MEMSZ] = header[AOUT_HE_DATA_FILESZ] = 0;
        header[AOUT_HE_DATA_VADDR] = header[AOUT_HE_TEXT_VADDR] + header[AOUT_HE_TEXT_MEMSZ];
        dataBuf = NULL;
    }

    header[AOUT_HE_TEXT_OFFSET] = 0;
    header[AOUT_HE_DATA_OFFSET] = header[AOUT_HE_TEXT_FILESZ];
#endif

    // Scan sections and copy data to a.out segments
    Elf_Scn* sd;
    forEachSection(sd, edata->elf) {
        Elf32_Shdr* sh = elf32_getshdr(sd);
        if (sh == NULL)
            elfError("Not a regular ELF file");
        if (sh->sh_type == SHT_PROGBITS && (sh->sh_flags & SHF_ALLOC)) {
            uint8_t* buf;
            if (sh->sh_addr >= header[AOUT_HE_DATA_VADDR]){
                buf = dataBuf + (sh->sh_addr - header[AOUT_HE_DATA_VADDR]);
            }
            else {
                buf = textBuf + (sh->sh_addr - header[AOUT_HE_TEXT_VADDR]);
            }

            Elf_Data* data;
            forEachSectionData(sd, data) {
                std::memcpy(buf, data->d_buf, data->d_size);
                buf += data->d_size;
            }
        }
    }

    // Write out the .aout header
    uint32_t* headerBuf = (uint32_t*) textBuf;
    for (size_t i = 0; i < N_AOUT_HDR_ENT; i++) {
        if (headerBuf[i])
            elfError("No space for a.out header");
        headerBuf[i] = toTargetEndian(header[i], edata->elfHeader);
    }

    *dBuf = dataBuf;
    *tBuf = textBuf;

    return 0;
}

#ifndef MKDEV_BUILD
static int readStab(e2adata *edata, char **error, Word asid, SymbolTable **rstab)
{
    static const char* const typeName[] = { "", "OBJ", "FUN" };
    static const char* const bindingName[] = { "LOC", "GLB", "WK " };

    // Write symbol records
    uint32_t funCount = 0, objCount = 0;
    elfSymbolTableIterator it(edata->elf), end;
    for (; it != end; ++it) {
        elfSymbol s = *it;
        unsigned char type = ELF32_ST_TYPE(s.details->st_info);
        if (!s.name.empty() &&
                (type == STT_FUNC || type == STT_OBJECT) &&
                (s.name[0] != '_' || s.name == "__start"))
        {
            if (type == STT_FUNC)
                funCount++;
            else
                objCount++;
        }
    }
    elfSymbolTableIterator itb(edata->elf);
    SymbolTable *stab = new SymbolTable(asid, funCount, objCount);
    char *typeStr = new char[31];
    int err = 0;

    for (; itb != end; ++itb) {
        elfSymbol s = *itb;
        unsigned char type = ELF32_ST_TYPE(s.details->st_info);
        unsigned char binding = ELF32_ST_BIND(s.details->st_info);
        if (!s.name.empty() &&
                (type == STT_FUNC || type == STT_OBJECT) &&
                (s.name[0] != '_' || s.name == "__start"))
        {
            sprintf(typeStr, "%s:0x%.8lX:0x%.8lX:%s",
                    typeName[type],
                    (unsigned long) s.details->st_value,
                    (unsigned long) s.details->st_size,
                    bindingName[binding]);

            err += stab->addSymbol(s.name.c_str(), typeStr);
        }
    }

    err = stab->finalizeSymbolTable(err);
    if(err){
        elfError("Invalid symbol table file format");
    }

    *rstab = stab;
    return 0;
}

biosElf::biosElf(const char *fname){
    address = 0;
    error = (char *) "";
    bool err = false;
    if(!initf(fname, &edata, &error)){

        // Check ELF object type
        if (edata.elfHeader->e_type != ET_REL)
            error = (char *) "Not a relocatable ELF object file";
        else{

            // Find .text section
            Elf_Scn* sd;
            forEachSection(sd, edata.elf) {
                Elf32_Shdr* sh = elf32_getshdr(sd);
                if (sh == NULL){
                    error = (char *) "Not a regular ELF file";
                    err = true;
                    break;
                }
                if ((sh->sh_type == SHT_PROGBITS) &&
                        (sh->sh_flags & SHF_ALLOC) &&
                        (sh->sh_flags & SHF_EXECINSTR))
                {
                    break;
                }
            }
            if(!err){
                if (sd == NULL)
                    error = (char *) "Could not find .text section";
                else{

                    Elf_Data* data;

                    // Copy .text into file
                    forEachSectionData(sd, data) {
                        if (data->d_type == ELF_T_BYTE) {
                            size = data->d_size;
                            buf = (uint8_t *)data->d_buf;
                        }
                    }
                }
            }
        }
    }
}

coreElf::coreElf(const char *fname, Word asid, const char *stabFname){
    daddr = taddr = 0;
    dsize = tsize = 0;
    error = (char *) "";
    if(!initf(fname, &edata, &error)){
        if(!readCore(&edata, &error, this, &dbuf, &tbuf, &dsize, &tsize)){
            closeElf();
            if(!initf(stabFname, &edata, &error))
                readStab(&edata, &error, asid, &stab);
        }
    }
}

coreElf::coreElf(const char *fname, Word asid){
    daddr = taddr = 0;
    dsize = tsize = 0;
    error = (char *) "";
    if(!initf(fname, &edata, &error)){
        if(!readCore(&edata, &error, this, &dbuf, &tbuf, &dsize, &tsize)){
            readStab(&edata, &error, asid, &stab);
        }
    }
}
#else
coreElf::coreElf(const char *fname){
    daddr = taddr = 0;
    dsize = tsize = 0;
    error = (char *) "";
    tread = false;
    if(!initf(fname, &edata, &error))
        readCore(&edata, &error, this, &dbuf, &tbuf, &dsize, &tsize);
}

int coreElf::read(void *dest, unsigned int size, unsigned int count){
    unsigned int i;
    uint8_t *tp = (uint8_t *) dest;
    for(i = 0; i < (size * count); i++, tp++){
        if(tread){ //read .data
            if(daddr < dsize){
                *tp = dbuf[daddr++];
            } else
                break;
        } else { //read .text
            if(taddr < tsize){
                *tp = tbuf[taddr++];
            } else {
                tread = true;
                i--;
                tp--;
            }
        }
    }

    return i;

}

#endif

coreElf::~coreElf(){
    if(dbuf != NULL)
        delete[](dbuf);
    if(tbuf != NULL)
        delete[](tbuf);
}

void elfLoader::closeElf(){
    elf_end(edata.elf);
    close(edata.file);
}