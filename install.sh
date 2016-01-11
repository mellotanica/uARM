#!/bin/bash

PREF="/usr/local"

ICONSD=$PREF"/lib/uarm/icons"
mkdir -p "$ICONSD"
cp icons/* "$ICONSD"

INCLUDED=$PREF"/include/uarm"
TESTD=$PREF"/share/doc/uarm/examples"
LDSCRIPTSD=$INCLUDED"/ldscripts"
DEFAULTD="/etc/default"
BIND=$PREF"/bin"

mkdir -p "$INCLUDED"
mkdir -p "$TESTD"
mkdir -p "$LDSCRIPTSD"

cp ldscript/* "$LDSCRIPTSD"
cp test/* "$TESTD"
cp facilities/* "$INCLUDED"

if test $OS == "Linux"; then
	cp uarm elf2uarm uarm-mkdev "$BIND"
else
	cp -r uarm.app /applications
	cp elf2uarm "$BIND"
fi

cp default/* "$DEFAULTD"
