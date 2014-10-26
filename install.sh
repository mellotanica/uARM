#!/bin/bash

if [ `id -u` != "0" ]; then
	echo "You must be root to run this script" 1>&2
	exit 1
fi

PREF="/usr"

if [ "$1" == "-d" ]; then
  PREF="$2"
  echo "$2" >> custom_install_directory
fi

#ICONSD=$PREF"/lib/uarm/icons"
INCLUDED=$PREF"/include/uarm"
TESTD=$PREF"/share/doc/uarm/examples"
LDSCRIPTSD=$INCLUDED"/ldscripts"
DEFAULTD="/etc/default"
BIND=$PREF"/bin"

#mkdir -p "$ICONSD"
mkdir -p "$INCLUDED"
mkdir -p "$TESTD"
mkdir -p "$LDSCRIPTSD"

#cp icons/* "$ICONSD"
cp ldscript/* "$LDSCRIPTSD"
cp test/* "$TESTD"
cp facilities/* "$INCLUDED"

cp uarm elf2uarm "$BIND"

cp default/* "$DEFAULTD"

exit 0
