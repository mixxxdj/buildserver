#!/bin/bash

# Echo commands
set -x

# Quit on errors
set -e

# Get the path to our scripts folder.
pushd `dirname $0` > /dev/null
PROGDIR=`pwd -P`
popd > /dev/null

export VERSION_NUMBER=1.0.25
export VERSION=libsndfile-${VERSION_NUMBER}
export ARCHIVE=$VERSION.tar.gz

echo "Building $VERSION for $MIXXX_ENVIRONMENT_NAME for architectures: ${MIXXX_ARCHS[@]}"

# You may need to change these from version to version.
export DYLIB=src/.libs/libsndfile.1.dylib
export STATICLIB=src/.libs/libsndfile.a

for ARCH in ${MIXXX_ARCHS[@]}
do
  mkdir -p $VERSION-$ARCH
  tar -zxf $DEPENDENCIES/$ARCHIVE -C $VERSION-$ARCH --strip-components 1
  cd $VERSION-$ARCH
  # libsndfile references Carbon.h
  curl https://gist.githubusercontent.com/metabr/8623583/raw/90966b76c6f52e1293b5303541e1f2d72e2afd08/0001-libsndfile-doesn-t-find-Carbon.h-using-XCode-4.3.patch | patch -p1
  source $PROGDIR/environment.sh $ARCH
  ./configure --host $HOST --target $TARGET --disable-dependency-tracking --prefix=$MIXXX_PREFIX
  make
  cd ..
done

# Install the i386 version in case there are binaries we want to run (our host is i386)
export ARCH=i386
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
