#ifndef ELF2ARM_H
#define ELF2ARM_H

#include "services/symbol_table.h"
#include "armProc/bus.h"
#include "armProc/aout.h"

#ifdef MACOS_BUILD
#include <libelf/libelf.h>
#include <libelf/gelf.h>
#else
#include <libelf.h>
#include <gelf.h>
#endif

typedef struct {
    Elf *elf;
    Elf32_Ehdr* elfHeader;
    int file;
} e2adata;

class elfLoader{
public:
    ~elfLoader() {closeElf();}
    void closeElf();

    bool allRight() {return !strcmp(error, "");}
    const char *getError(){return error;}

protected:
    e2adata edata;
    char *error;
};

class biosElf : public elfLoader{

public:
    biosElf(const char *fname);

    Word getSize() {return size;}

    Byte readByte() {return (address < size ? buf[address++] : 0);}

private:
    int address;
    Word size;
    uint8_t *buf;
};

class coreElf : public elfLoader{

public:
    coreElf(const char *fname, Word asid);
    coreElf(const char *fname, Word asid, const char *stabFname);
    ~coreElf();

    Word getDataSize() {return dsize;}
    Word getTextSize() {return tsize;}

    void seekText() {taddr = 0;}

    Byte readDataByte() {return (daddr < dsize ? dbuf[daddr++] : 0);}
    Byte readTextByte() {return (taddr < tsize ? tbuf[taddr++] : 0);}

    uint32_t header[N_AOUT_HDR_ENT];

    SymbolTable *getSymbolTable() {return stab;}
private:
    Word dsize, tsize;
    int daddr, taddr;
    uint8_t *dbuf, *tbuf;
    SymbolTable *stab;
};

#endif // ELF2ARM_H
