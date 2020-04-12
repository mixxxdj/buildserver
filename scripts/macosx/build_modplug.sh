#!/bin/bash

# Echo commands
set -x

# Quit on errors
set -e

# Get the path to our scripts folder.
pushd `dirname $0` > /dev/null
PROGDIR=`pwd -P`
popd > /dev/null

export MODPLUG_VERSION_NUMBER=0.8.9.0
export MODPLUG_VERSION=libmodplug-${MODPLUG_VERSION_NUMBER}
export MODPLUG_ARCHIVE=$MODPLUG_VERSION.tar.gz

echo "Building $MODPLUG_VERSION for $MIXXX_ENVIRONMENT_NAME for architectures: ${MIXXX_ARCHS[@]}"

# You may need to change these from version to version.
export MODPLUG_DYLIB=libmodplug/.libs/libmodplug.1.dylib
export MODPLUG_STATICLIB=libmodplug/.libs/libmodplug.a

for ARCH in ${MIXXX_ARCHS[@]}
do
  mkdir -p $MODPLUG_VERSION-$ARCH
  tar -zxf $DEPENDENCIES/$MODPLUG_ARCHIVE -C $MODPLUG_VERSION-$ARCH --strip-components 1
  cd $MODPLUG_VERSION-$ARCH
  source $PROGDIR/environment.sh $ARCH
  ./configure --host $HOST --target $TARGET --disable-dependency-tracking --prefix=$MIXXX_PREFIX --enable-float-approx
  make clean
  make
  cd ..
done

# Install the host version in case there are binaries we want to run.
export ARCH=$HOST_ARCH
cd $MODPLUG_VERSION-$ARCH
source $PROGDIR/environment.sh $ARCH

OTHER_MODPLUG_DYLIBS=()
OTHER_MODPLUG_STATICLIBS=()
for OTHER_ARCH in ${MIXXX_ARCHS[@]}
do
  if [ $OTHER_ARCH != $ARCH ]; then
    OTHER_MODPLUG_DYLIBS+=(../$MODPLUG_VERSION-$OTHER_ARCH/$MODPLUG_DYLIB)
    OTHER_MODPLUG_STATICLIBS+=(../$MODPLUG_VERSION-$OTHER_ARCH/$MODPLUG_STATICLIB)
  fi
done

lipo -create ./$MODPLUG_DYLIB ${OTHER_MODPLUG_DYLIBS[@]} -output ./$MODPLUG_DYLIB
lipo -create ./$MODPLUG_STATICLIB ${OTHER_MODPLUG_STATICLIBS[@]} -output ./$MODPLUG_STATICLIB
make install
cd ..
