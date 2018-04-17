#!/bin/bash

# Echo commands
set -x

# Quit on errors
set -e

# Get the path to our scripts folder.
pushd `dirname $0` > /dev/null
PROGDIR=`pwd -P`
popd > /dev/null

export LAME_VERSION_NUMBER=3.100
export LAME_VERSION=lame-${LAME_VERSION_NUMBER}
export LAME_ARCHIVE=$LAME_VERSION.tar.gz

echo "Building $LAME_VERSION for $MIXXX_ENVIRONMENT_NAME for architectures: ${MIXXX_ARCHS[@]}"

# You may need to change these from version to version.
export LAME_DYLIB=libmp3lame/.libs/libmp3lame.0.dylib
export LAME_STATICLIB=libmp3lame/.libs/libmp3lame.a

for ARCH in ${MIXXX_ARCHS[@]}
do
  mkdir -p $LAME_VERSION-$ARCH
  tar -zxf $DEPENDENCIES/$LAME_ARCHIVE -C $LAME_VERSION-$ARCH --strip-components 1
  cd $LAME_VERSION-$ARCH
  source $PROGDIR/environment.sh $ARCH
  # Drop non-existent symbol.
  # https://hydrogenaud.io/index.php/topic,114777.0.html
  sed -i'' -e '/lame_init_old/d' include/libmp3lame.sym
  ./configure --host $HOST --target $TARGET --disable-dependency-tracking --prefix=$MIXXX_PREFIX --enable-float-approx
  make clean
  make
  cd ..
done

# Install the host version in case there are binaries we want to run.
export ARCH=$HOST_ARCH
cd $LAME_VERSION-$ARCH
source $PROGDIR/environment.sh $ARCH

OTHER_LAME_DYLIBS=()
OTHER_LAME_STATICLIBS=()
for OTHER_ARCH in ${MIXXX_ARCHS[@]}
do
  if [ $OTHER_ARCH != $ARCH ]; then
    OTHER_LAME_DYLIBS+=(../$LAME_VERSION-$OTHER_ARCH/$LAME_DYLIB)
    OTHER_LAME_STATICLIBS+=(../$LAME_VERSION-$OTHER_ARCH/$LAME_STATICLIB)
  fi
done

lipo -create ./$LAME_DYLIB ${OTHER_LAME_DYLIBS[@]} -output ./$LAME_DYLIB
lipo -create ./$LAME_STATICLIB ${OTHER_LAME_STATICLIBS[@]} -output ./$LAME_STATICLIB
make install
cd ..
