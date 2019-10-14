#!/bin/bash

# Echo commands
set -x

# Quit on errors
set -e

# make sure to get the path right
pushd `dirname $0` > /dev/null
PROGDIR=`pwd -P`
popd > /dev/null

export VERSION_NUMBER=2.5.0
export VERSION=scons-${VERSION_NUMBER}
export ARCHIVE=$VERSION.tar.gz

tar -zxf $DEPENDENCIES/$ARCHIVE
cd $VERSION

source $PROGDIR/environment.sh

python setup.py build
python setup.py install --prefix=$MIXXX_PREFIX
