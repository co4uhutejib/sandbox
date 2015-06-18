#!/bin/sh

ROOT=$(git rev-parse --show-toplevel)
BIN=$ROOT/bin
UTILS=$ROOT/utils
MAKE=make

mkdir -p $BIN

PLATFORM=`uname -s`
if [[ "$PLATFORM" == "mingw"* ]]; then
	PLATFORM="windows"
elif [[ "$PLATFORM" == "MINGW32"* ]]; then
	PLATFORM="windows"
fi

TARGET=gmake
EXE=""

if [[ "$PLATFORM" == "windows" ]]; then
	EXE=".exe"
	MAKE=echo
	TARGET=vs2013
fi

BOOTSTRAPPREMAKE=$BIN/premake5$EXE

if [ ! -f "$BOOTSTRAPPREMAKE" ]; then
	BOOTSTRAPPREMAKE=$BIN/premake4$EXE
fi

if [ ! -f "$BOOTSTRAPPREMAKE" ]; then
	echo "not found any premake for bootstrap, place it to $BIN"
	if [[ "$PLATFORM" != "windows" ]]; then
		BOOTSTRAPPREMAKE=$BIN/premake5
		echo "try get it"
		cd $BIN && curl -L "https://github.com/premake/premake-core/releases/download/v5.0.0.alpha4/premake-5.0.0.alpha4-macosx.tar.gz" | tar xz 
	fi
	exit 1
fi

echo "rebuild premake5"
cd $UTILS/premake5
$BOOTSTRAPPREMAKE embed || exit 1
$BOOTSTRAPPREMAKE $TARGET || exit 1
$MAKE config=release || exit 1
cp $UTILS/premake5/bin/release/* $BIN
PREMAKE5=$BIN/premake5$EXE
cd $UTILS

echo "rebuild assetsbuilder"
cd $UTILS/assetsbuilder
$PREMAKE5 --scripts=$ROOT/projects/premake5 $TARGET || exit 1
$MAKE config=release || exit 1
cp $UTILS/assetsbuilder/bin/release/* $BIN
cd $UTILS