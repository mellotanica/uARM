#!/bin/bash

if [ `id -u` != "0" ]; then
	echo "You must be root to run this script" 1>&2
	exit 1
fi

ICONSD="/usr/lib/uarm/icons"
INCLUDED="/usr/include/uarm"
TESTD="/usr/share/doc/uarm/examples"
LDSCRIPTSD=INCLUDED"/ldscript"
DEFAULTD="/etc/default"
BIND="/usr/bin"

mkdir -p "$ICONSD"
mkdir -p "$INCLUDED"
mkdir -p "$TESTD"
mkdir -p "$LDSCRIPTSD"

cp icons/* "$ICONSD"
cp ldscript/* "$LDSCRIPTSD"
cp test/* "$TESTD"
cp facilities/* "$INCLUDED"

cp uarm elf2uarm "$BIND"

cp default/* "$DEFAULTD"

exit 0
