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
#include "services/elf2arm_mkdev.h"
#include "armProc/blockdev_params.h"
#include "facilities/arch.h"


/*
 * Functions throughtout this module close over this global!
 */


static const size_t kBlockSize = 4096;

#define forEachSection(sd, elf) \
    for (sd = elf_nextscn(elf, NULL); sd != NULL; sd = elf_nextscn(elf, sd))

#define forEachSectionData(sd, data) \
    for (data = elf_getdata(sd, NULL); data != NULL; data = elf_getdata(sd, data))

#define elfError(emsg) ({*error = (char *) emsg; return 1;})


uint32_t toTargetEndian(uint32_t x);

const char* programName;

// Input file name
const char* fileName = NULL;

// Be verbose
// static bool verbose = false;


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
