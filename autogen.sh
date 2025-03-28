#!/bin/sh

if [ ! -d "m4" ]; then
	mkdir m4
fi

if [ ! -d "build-aux" ]; then
	mkdir build-aux
fi

autoreconf -fvi
