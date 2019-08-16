#!/bin/bash

# Echo commands
set -x

# Quit on errors
set -e

# Get the path to our scripts folder.
pushd `dirname $0` > /dev/null
PROGDIR=`pwd -P`
popd > /dev/null

export VERSION_NUMBER=2.4.1
export VERSION=libshout-idjc-${VERSION_NUMBER}
export ARCHIVE=$VERSION.tar.gz

echo "Building $VERSION for $MIXXX_ENVIRONMENT_NAME for architectures: ${MIXXX_ARCHS[@]}"

# You may need to change these from version to version.
export DYLIB=src/.libs/libshout-idjc.3.dylib
export STATICLIB=src/.libs/libshout-idjc.a

for ARCH in ${MIXXX_ARCHS[@]}
do
  mkdir -p $VERSION-$ARCH
  tar -zxf $DEPENDENCIES/$ARCHIVE -C $VERSION-$ARCH --strip-components 1
  cd $VERSION-$ARCH

  # Apply patch fixing: 
  # https://trac.xiph.org/ticket/2244
  # https://bugs.launchpad.net/mixxx/+bug/1544739
  patch -p1 < $PROGDIR/fix_libshout_ticket2244.patch

  source $PROGDIR/environment.sh $ARCH
  ./configure --host $HOST --target $TARGET --disable-dependency-tracking --prefix=$MIXXX_PREFIX
  make
  cd ..
done

# Install the host version in case there are binaries we want to run.
export ARCH=$HOST_ARCH
cd $VERSION-$ARCH
source $PROGDIR/environment.sh $ARCH

OTHER_DYLIBS=()
OTHER_STATICLIBS=()
for OTHER_ARCH in ${MIXXX_ARCHS[@]}
do
  if [ $OTHER_ARCH != $ARCH ]; then
    OTHER_DYLIBS+=(../$VERSION-$OTHER_ARCH/$DYLIB)
    OTHER_STATICLIBS+=(../$VERSION-$OTHER_ARCH/$STATICLIB)
  fi
done

lipo -create ./$DYLIB ${OTHER_DYLIBS[@]} -output ./$DYLIB
lipo -create ./$STATICLIB ${OTHER_STATICLIBS[@]} -output ./$STATICLIB
make install
cd ..
