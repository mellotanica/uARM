#!/usr/bin/env python2

import sys

header_fields = {
        0 : "Header TAG",
        1 : "Entry Point",
        2 : ".text virtual address",
        3 : ".text memory size",
        4 : ".text offset in file",
        5 : ".text size in file",
        6 : ".data virtual address",
        7 : ".data memory size",
        8 : ".data offset in file",
        9 : ".data size in file"
}

header_size = 16

magic_numbers = {
    0x004D5241 : "Disk",
    0x014D5241 : "Tape",
    0x024D5241 : "BIOS",
    0x034D5241 : "Core",
    0x044D5241 : "aOut",
    0x414D5241 : "Symbol Table"
}

noheader_mn = [0x024D5241, 0x414D5241]
header_mn = [0x034D5241, 0x044D5241]

def usage():
    print sys.argv[0]
    print "    you must specify a valid .uarm file"
    sys.exit(1)

def noHeader(filetype):
    print "regular "+filetype+" file"
    sys.exit(0)

def readWord(fname):
    rw = ord(fname.read(1))
    rw = rw | (ord(fname.read(1)) << 8)
    rw = rw | (ord(fname.read(1)) << 16)
    rw = rw | (ord(fname.read(1)) << 24)
    return rw

if len(sys.argv) != 2:
    usage()

uarm = open(sys.argv[1], 'rb')

descriptor = readWord(uarm)

if not descriptor in magic_numbers.keys():
    usage()

if descriptor in noheader_mn:
    noHeader(magic_numbers[descriptor])
    
if not descriptor in header_mn:
    print "regular "+magic_numbers[descriptor]+" file"
    descriptor = readWord(uarm)

if not descriptor in header_mn:
    print "irregular file content"
    sys.exit(0)

print "regular "+magic_numbers[descriptor]+" field:"

i = 0
val = descriptor

while i < header_size:
    out = ''
    if i in header_fields:
        out = header_fields[i]
    else:
        out = "undefined field"
    print(out + ":\t" + str(format(val, 'x')))
    i = i+1
    val = readWord(uarm)

sys.exit(0)
