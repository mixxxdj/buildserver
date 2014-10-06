#!/usr/bin/env bash

# make sure to get the path right
PROGDIR="$( cd "$(dirname "$0")" ; pwd -P )"
source $PROGDIR/config.sh

for DIST in $DISTS; do
    for ARCH in $ARCHS; do
	sudo DIST=$DIST ARCH=$ARCH pbuilder update
    done;
done;
