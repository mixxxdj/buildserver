#!/usr/bin/env bash

# make sure to get the path right
PROGNAME=`basename $0`
PROGDIR=`dirname $0`
source $PROGDIR/config.sh

for DIST in $DISTS; do 
    for ARCH in $ARCHS; do 
	sudo DIST=$DIST ARCH=$ARCH pbuilder update 
    done; 
done;
