#!/bin/bash

# Echo commands
set -x

# Quit on errors
set -e

# Get the path to our scripts folder.
pushd `dirname $0` > /dev/null
PROGDIR=`pwd -P`
popd > /dev/null

export VERSION=hss1394-r8
export ARCHIVE=$VERSION.tar.gz

echo "Building $VERSION for $MIXXX_ENVIRONMENT_NAME for architectures: ${MIXXX_ARCHS[@]}"

# You may need to change these from version to version.
export DYLIB=obj/libhss1394.dylib

for ARCH in ${MIXXX_ARCHS[@]}
do
  mkdir -p $VERSION-$ARCH/scons
  tar -zxf $DEPENDENCIES/$ARCHIVE -C $VERSION-$ARCH --strip-components 1
  cd $VERSION-$ARCH
  tar -zxf $DEPENDENCIES/scons-2.5.0.tar.gz -C scons --strip-components 1
  # Build SCons
  cd scons && python setup.py build && cd ..
  source $PROGDIR/environment.sh $ARCH
  python scons/script/scons prefix=$MIXXX_PREFIX
  cd ..
done

# Install the $HOST_ARCH version in case there are binaries we want to run.
export ARCH=$HOST_ARCH
cd $VERSION-$ARCH
source $PROGDIR/environment.sh $ARCH

OTHER_DYLIBS=()
for OTHER_ARCH in ${MIXXX_ARCHS[@]}
do
  if [ $OTHER_ARCH != $ARCH ]; then
    OTHER_DYLIBS+=(../$VERSION-$OTHER_ARCH/$DYLIB)
  fi
done

lipo -create ./$DYLIB ${OTHER_DYLIBS[@]} -output ./$DYLIB
python scons/script/scons prefix=$MIXXX_PREFIX install
install_name_tool -id $MIXXX_PREFIX/lib/libhss1394.dylib $MIXXX_PREFIX/lib/libhss1394.dylib

cd ..
