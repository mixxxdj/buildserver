#!/bin/bash

# Echo commands
set -x

# Quit on errors
set -e

# make sure to get the path right
pushd `dirname $0` > /dev/null
PROGDIR=`pwd -P`
popd > /dev/null

export VERSION_NUMBER=3.15.4
export VERSION=cmake-${VERSION_NUMBER}
export ARCHIVE=$VERSION.tar.gz

tar -zxf $DEPENDENCIES/$ARCHIVE
cd $VERSION

source $PROGDIR/environment.sh
./bootstrap --prefix=$MIXXX_PREFIX && make && make install
