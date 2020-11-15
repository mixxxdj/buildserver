#!/bin/bash

# Echo commands
set -x

# Quit on errors
set -e

# make sure to get the path right
pushd `dirname $0` > /dev/null
PROGDIR=`pwd -P`
popd > /dev/null

export VERSION_NUMBER=1.16.2
export VERSION=automake-${VERSION_NUMBER}
export ARCHIVE=$VERSION.tar.xz

tar -zxf $DEPENDENCIES/$ARCHIVE
cd $VERSION

source $PROGDIR/environment.sh

./configure --prefix=$MIXXX_PREFIX && make && make install
