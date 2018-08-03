#! /bin/sh

TARGET_OS=`uname -s`
MHD_NAME=`uname -m`
echo $TARGET_OS
echo $MHD_NAME

if [ "$TARGET_OS" == "Darwin" ] && [ "$MHD_NAME" == "x86_64" ]; then
	echo "test: Darwin & X86_64"
else
	echo "test: not x86_64"
fi

