#!/bin/bash

if [ `id -u` != "0" ]; then
	echo "You must be root to run this script" 1>&2
	exit 1
fi

PREF="/usr"

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
if [ -d `dirname "$ICONSD"` ]
then
  rmdir --ignore-fail-on-non-empty `dirname "$ICONSD"`
fi
rm -rf "$INCLUDED"
if [ -d `dirname "$INCLUDED"` ]
then
  rmdir --ignore-fail-on-non-empty `dirname "$INCLUDED"`
fi
rm -rf "$TESTD"
if [ -d `dirname "$TESTD"` ]
then
  rmdir --ignore-fail-on-non-empty `dirname "$TESTD"`
fi

rm -f "$BIND"/uarm "$BIND"/elf2uarm "$BIND"/uarm-mkdev

cd default
for i in *
do
  rm -f "$DEFAULTD"/$i
done

exit 0
