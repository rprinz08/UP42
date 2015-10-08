#!/bin/bash

ARCH=$1
if [ -z $ARCH ]; then
	ARCH=ia32
fi

TARGET=$2
if [ -z $TARGET ]; then
	TARGET=all
fi

make -f Makefile.linux $TARGET

echo "Done"
