#!/bin/bash

MODVER=$1

if [ "$MODVER" == "" ] ; then
	ls content/lib/modules/
	exit 0
fi

if [ ! -d content/lib/modules/$MODVER ]; then
	ls content/lib/modules/
	exit 0
fi

tar -cjvpf $MODVER.tar.bz2 content/lib/modules/$MODVER

[ -f $MODVER.tar.bz2 ] && rm -rf content/lib/modules/$MODVER
