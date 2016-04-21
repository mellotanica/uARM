#!/bin/bash

if [ `id -u` != "0" ]; then
    echo "You must be root to run this script" 1>&2
    exit 1
fi

if [ `uname` == "Linux" ]; then
    PREF="/usr"
else
    PREF="/usr/local"
fi

if [ "$1" == "-d" ]; then
  PREF="$2"
fi

ICONSD=$PREF"/lib/uarm/icons"
INCLUDED=$PREF"/include/uarm"
TESTD=$PREF"/share/doc/uarm/examples"
LDSCRIPTSD=$INCLUDED"/ldscripts"
DEFAULTD="/etc/default"
BIND=$PREF"/bin"

rm -rf "$ICONSD"
rm -rf "$INCLUDED"
rm -rf "$TESTD"
rm -f "$BIND"/uarm "$BIND"/elf2uarm "$BIND"/uarm-mkdev "$BIND"/uarm-readuarm

if [ `uname` == "Darwin" ]; then
	rm -rf /applications/uarm.app
fi

cd default
for i in *
do
  rm -f "$DEFAULTD"/$i
done
