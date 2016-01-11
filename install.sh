#!/bin/bash

PREF="/usr/local"
ICONSD=$PREF"/lib/uarm/icons"
INCLUDED=$PREF"/include/uarm"
TESTD=$PREF"/share/doc/uarm/examples"
LDSCRIPTSD=$INCLUDED"/ldscripts"
DEFAULTD="/etc/default"
BIND=$PREF"/bin"

mkdir -p "$ICONSD"
mkdir -p "$INCLUDED"
mkdir -p "$TESTD"
mkdir -p "$LDSCRIPTSD"

cp icons/* "$ICONSD"
cp ldscript/* "$LDSCRIPTSD"
cp test/* "$TESTD"
cp facilities/* "$INCLUDED"
cp elf2uarm "$BIND"
cp default/* "$DEFAULTD"
