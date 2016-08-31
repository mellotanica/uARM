#ifndef ELF2ARM_H
#define ELF2ARM_H

#include "services/symbol_table.h"
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
    unsigned int address;
    Word size;
    uint8_t *buf;
};

class coreElf : public elfLoader{

public:
    ~coreElf();

#ifndef MKDEV_BUILD
    coreElf(const char *fname, Word asid);
    coreElf(const char *fname, Word asid, const char *stabFname);

    Word getDataSize() {return dsize;}
    Word getTextSize() {return tsize;}

    void seekText() {taddr = 0;}

    Byte readDataByte() {return (daddr < dsize ? dbuf[daddr++] : 0);}
    Byte readTextByte() {return (taddr < tsize ? tbuf[taddr++] : 0);}

    SymbolTable *getSymbolTable() {return stab;}
#else
    coreElf(const char *fname);

    int read(void *dest, unsigned int size, unsigned int count);
    bool eof() { return (daddr >= dsize && taddr >= tsize); }
#endif

    uint32_t header[N_AOUT_HDR_ENT];
private:
    Word dsize, tsize;
    unsigned int daddr, taddr;
    uint8_t *dbuf, *tbuf;
#ifndef MKDEV_BUILD
    SymbolTable *stab;
#else
    bool tread;
#endif
};

#endif // ELF2ARM_H
