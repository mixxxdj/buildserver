#!/bin/bash

# Echo commands
set -x

# Quit on errors
set -e

# Get the path to our scripts folder.
pushd `dirname $0` > /dev/null
PROGDIR=`pwd -P`
popd > /dev/null

echo "Building fdk-aac for $MIXXX_ENVIRONMENT_NAME for architectures: ${MIXXX_ARCHS[@]}"

source $PROGDIR/environment.sh ${MIXXX_ARCHS[0]}
cd $DEPENDENCIES/fdk-aac
./autogen.sh
./configure --prefix=$MIXXX_PREFIX --host $HOST --target $TARGET
make
make install
cd ..

