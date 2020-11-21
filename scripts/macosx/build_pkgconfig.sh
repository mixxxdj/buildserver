#!/bin/bash

# Echo commands
set -x

# Quit on errors
set -e

# make sure to get the path right
pushd `dirname $0` > /dev/null
PROGDIR=`pwd -P`
popd > /dev/null

export VERSION_NUMBER=0.29.2
export VERSION=pkg-config-${VERSION_NUMBER}
export ARCHIVE=$VERSION.tar.gz

tar -zxf $DEPENDENCIES/$ARCHIVE
cd $VERSION

source $PROGDIR/environment.sh

./configure --prefix=$MIXXX_PREFIX --with-internal-glib && make && make install
