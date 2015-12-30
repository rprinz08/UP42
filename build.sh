#!/bin/bash

echo "ARGS: $1 $2"

TARGET=$1
if [ -z $TARGET ]; then
	TARGET=all
fi

if [ ! -z $2 ]; then
    ARCH=$2
    export ARCH
fi

make -e -f Makefile.linux $TARGET

echo "Done"
