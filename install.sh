#!/bin/bash

if [ `id -u` != "0" ]; then
	echo "You must be root to run this script" 1>&2
	exit 1
fi

ICONSD="/usr/lib/uarm/icons"
INCLUDED="/usr/include/uarm"
TESTD=INCLUDED"/test"
LDSCRIPTSD=INCLUDED"/ldscripts"

mkdir -p "$ICONSD"
mkdir -p "$INCLUDED"
mkdir -p "$TESTD"
mkdir -p "$LDSCRIPTSD"

cp icons/* "$ICONSD"
cp ldscripts/* "$LDSCRIPTS"
cp test/* "$TESTD"
cp facilities/* "$INCLUDED"

cp uarm elf2uarm /usr/bin

exit 0
