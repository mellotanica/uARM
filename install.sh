#!/bin/bash

if [ `id -u` != "0" ]; then
	echo "You must be root to run this script" 1>&2
	exit 1
fi

OS=`uname -s`

DESTDIR=""

if test $OS == "Linux"; then
	PREF="/usr"
else
	PREF="/usr/local"
fi
ICONSONLY="false"

while [ -n "$1" ]; do
	case "$1" in
		"-d")
			PREF="$2"
			echo "$2" >> custom_install_directory
			shift
			;;
		"-i")
			ICONSONLY="true"
			;;
		"-b")
			DESTDIR="$2"
			PREF="$DESTDIR""$PREF"
			shift
			;;
		"*")
			echo "unknown option $1, skipping"
			;;
	esac
	shift
done

ICONSD=$PREF"/lib/uarm/icons"
mkdir -p "$ICONSD"
cp icons/* "$ICONSD"
RED='\033[0;31m'
NC='\033[0m'
HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

pushd "$HERE" > /dev/null
if [ "$ICONSONLY" == "false" ]; then
	INCLUDED=$PREF"/include/uarm"
	TESTD=$PREF"/share/doc/uarm/examples"
	LDSCRIPTSD=$INCLUDED"/ldscripts"
	DEFAULTD="$DESTDIR/etc/default"
	BIND=$PREF"/bin"

	mkdir -p "$INCLUDED"
	mkdir -p "$TESTD"
	mkdir -p "$LDSCRIPTSD"
	mkdir -p "$DEFAULTD"
	mkdir -p "$BIND"

	cp ldscript/* "$LDSCRIPTSD"
	cp test/* "$TESTD"
	cp facilities/* "$INCLUDED"

	if test $OS == "Linux"; then
		cp uarm "$BIND"
	else
		cp -r uarm.app /applications
	fi
	cp elf2uarm uarm-mkdev "$BIND"
	cp -f "$HERE"/facilities/readuarm.py "$BIND"/uarm-readuarm
	
	for d in default/*; do
		f=`basename "$d"`
		if test -f "$DEFAULTD/$f"; then
			printf "${RED}WARNING${NC}: $DEFAULTD/$f exists, not overwriting.. check $HERE/$d for new options\n"
		else
			cp "$d" "$DEFAULTD"
		fi
	done
fi
popd > /dev/null

exit 0
