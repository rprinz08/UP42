#!/bin/bash

TARGET=$1
if [ -z $TARGET ]; then
	TARGET=all
fi

make -f Makefile.linux $TARGET

