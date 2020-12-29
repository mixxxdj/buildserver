#!/bin/bash

# Echo commands
set -x

# Quit on errors
set -e

# Get the path to our scripts folder.
pushd `dirname $0` > /dev/null
PROGDIR=`pwd -P`
popd > /dev/null

export VERSION_NUMBER=3.3.9
export VERSION=fftw-${VERSION_NUMBER}
export ARCHIVE=$VERSION.tar.gz

echo "Building $VERSION for $MIXXX_ENVIRONMENT_NAME for architectures: ${MIXXX_ARCHS[@]}"

for ARCH in ${MIXXX_ARCHS[@]}
do
  mkdir -p $VERSION-$ARCH
  tar -zxf $DEPENDENCIES/$ARCHIVE -C $VERSION-$ARCH --strip-components 1
  cd $VERSION-$ARCH
  source $PROGDIR/environment.sh $ARCH
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$MIXXX_PREFIX" -DCMAKE_OSX_DEPLOYMENT_TARGET="$MIXXX_MACOSX_TARGET" -DCMAKE_OSX_SYSROOT="$SDKROOT" -DCMAKE_VERBOSE_MAKEFILE=TRUE -DBUILD_SHARED_LIBS=TRUE
  cmake --build build --target install
  cd ..
done
