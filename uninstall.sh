#!/bin/bash

PREF="/usr/local"
ICONSD=$PREF"/lib/uarm/icons"
INCLUDED=$PREF"/include/uarm"
TESTD=$PREF"/share/doc/uarm/examples"
LDSCRIPTSD=$INCLUDED"/ldscripts"
DEFAULTD="/etc/default"
BIND=$PREF"/bin"

rm -rf "$ICONSD"
rm -rf "$INCLUDED"
rm -rf "$TESTD"
rm -f "$BIND"/elf2uarm

cd default
for i in *
do
  rm -f "$DEFAULTD"/$i
done
