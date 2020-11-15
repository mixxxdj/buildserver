#!/bin/bash

# Echo commands
set -x

# Quit on errors
set -e

# Get the path to our scripts folder.
pushd `dirname $0` > /dev/null
PROGDIR=`pwd -P`
popd > /dev/null

export VERSION_NUMBER=217
export VERSION=portmidi-src-${VERSION_NUMBER}
export ARCHIVE=$VERSION.zip

echo "Building $VERSION for $MIXXX_ENVIRONMENT_NAME for architectures: ${MIXXX_ARCHS[@]}"

# You may need to change these from version to version.
export DYLIB=libportmidi.dylib
export STATICLIB=libportmidi_s.a

for ARCH in ${MIXXX_ARCHS[@]}
do
  unzip -o $DEPENDENCIES/$ARCHIVE -d $VERSION-$ARCH
  cd $VERSION-$ARCH/portmidi/
  source $PROGDIR/environment.sh $ARCH
  curl https://gist.githubusercontent.com/rryan/3b92d65242d358d1fb94/raw/e345fdaa635f3afa1ce21c4253782133379b74f0/portmidi.patch | patch -p1

  cmake . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$MIXXX_PREFIX" -DCMAKE_OSX_DEPLOYMENT_TARGET="$MACOSX_DEPLOYMENT_TARGET" -DCMAKE_OSX_SYSROOT="$SDKROOT" -DCMAKE_VERBOSE_MAKEFILE=TRUE -DCMAKE_OSX_ARCHITECTURES="$ARCH"
  make
  cd ../..
done

# Install the $HOST_ARCH version in case there are binaries we want to run.
export ARCH=$HOST_ARCH
cd $VERSION-$ARCH/portmidi/
source $PROGDIR/environment.sh $ARCH

OTHER_DYLIBS=()
OTHER_STATICLIBS=()
for OTHER_ARCH in ${MIXXX_ARCHS[@]}
do
  if [ $OTHER_ARCH != $ARCH ]; then
    OTHER_DYLIBS+=(../../$VERSION-$OTHER_ARCH/portmidi/$DYLIB)
    OTHER_STATICLIBS+=(../../$VERSION-$OTHER_ARCH/portmidi/$STATICLIB)
  fi
done

lipo -create ./$DYLIB ${OTHER_DYLIBS[@]} -output ./$DYLIB
lipo -create ./$STATICLIB ${OTHER_STATICLIBS[@]} -output ./$STATICLIB
make install
install_name_tool -id $MIXXX_PREFIX/lib/libportmidi.dylib $MIXXX_PREFIX/lib/libportmidi.dylib

cd ../..
