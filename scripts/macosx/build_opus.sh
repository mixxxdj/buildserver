#!/bin/bash

# Echo commands
set -x

# Quit on errors
set -e

# Get the path to our scripts folder.
pushd `dirname $0` > /dev/null
PROGDIR=`pwd -P`
popd > /dev/null

export OPUS_VERSION_NUMBER=1.1
export OPUS_VERSION=opus-${OPUS_VERSION_NUMBER}
export OPUS_ARCHIVE=$OPUS_VERSION.tar.gz

echo "Building $OPUS_VERSION for $MIXXX_ENVIRONMENT_NAME for architectures: ${MIXXX_ARCHS[@]}"

# You may need to change these from version to version.
export OPUS_DYLIB=.libs/libopus.0.dylib
export OPUS_STATICLIB=.libs/libopus.a

for ARCH in ${MIXXX_ARCHS[@]}
do
  mkdir -p $OPUS_VERSION-$ARCH
  tar -zxf $DEPENDENCIES/$OPUS_ARCHIVE -C $OPUS_VERSION-$ARCH --strip-components 1
  cd $OPUS_VERSION-$ARCH
  source $PROGDIR/environment.sh $ARCH
  ./configure --host $HOST --target $TARGET --disable-dependency-tracking --prefix=$MIXXX_PREFIX
  make clean
  make
  cd ..
done

# Install the i386 version in case there are binaries we want to run (our host is i386)
export ARCH=i386
cd $OPUS_VERSION-$ARCH
source $PROGDIR/environment.sh $ARCH

OTHER_OPUS_DYLIBS=()
OTHER_OPUS_STATICLIBS=()
for OTHER_ARCH in ${MIXXX_ARCHS[@]}
do
  if [ $OTHER_ARCH != $ARCH ]; then
    OTHER_OPUS_DYLIBS+=(../$OPUS_VERSION-$OTHER_ARCH/$OPUS_DYLIB)
    OTHER_OPUS_STATICLIBS+=(../$OPUS_VERSION-$OTHER_ARCH/$OPUS_STATICLIB)
  fi
done

lipo -create ./$OPUS_DYLIB ${OTHER_OPUS_DYLIBS[@]} -output ./$OPUS_DYLIB
lipo -create ./$OPUS_STATICLIB ${OTHER_OPUS_STATICLIBS[@]} -output ./$OPUS_STATICLIB
make install
cd ..

export OPUSFILE_VERSION_NUMBER=0.6
export OPUSFILE_VERSION=opusfile-${OPUSFILE_VERSION_NUMBER}
export OPUSFILE_ARCHIVE=$OPUSFILE_VERSION.tar.gz

echo "Building $OPUSFILE_VERSION for $MIXXX_ENVIRONMENT_NAME for architectures: ${MIXXX_ARCHS[@]}"

# You may need to change these from version to version.
export OPUSFILE_DYLIB=.libs/libopusfile.0.dylib
export OPUSFILE_STATICLIB=.libs/libopusfile.a

for ARCH in ${MIXXX_ARCHS[@]}
do
  mkdir -p $OPUSFILE_VERSION-$ARCH
  tar -zxf $DEPENDENCIES/$OPUSFILE_ARCHIVE -C $OPUSFILE_VERSION-$ARCH --strip-components 1
  cd $OPUSFILE_VERSION-$ARCH
  # NOTE(rryan): opusfile has a bug (?) where it includes opus_multistream.h as
  # <opus_multistream.h> while the file is typically installed to
  # opus/opus_multistream.h.
  sed -e 's/<opus_multistream.h>/<opus\/opus_multistream.h>/g' -i '' include/opusfile.h
  source $PROGDIR/environment.sh $ARCH
  DEPS_CFLAGS="-I$MIXXX_PREFIX/include" DEPS_LIBS="-L$MIXXX_PREFIX/lib -logg -lopus" ./configure --host $HOST --target $TARGET --disable-dependency-tracking --prefix=$MIXXX_PREFIX --enable-fixed-point --disable-http
  make clean
  make
  cd ..
done

# Install the i386 version in case there are binaries we want to run (our host is i386)
export ARCH=i386
cd $OPUSFILE_VERSION-$ARCH
source $PROGDIR/environment.sh $ARCH

OTHER_OPUSFILE_DYLIBS=()
OTHER_OPUSFILE_STATICLIBS=()
for OTHER_ARCH in ${MIXXX_ARCHS[@]}
do
  if [ $OTHER_ARCH != $ARCH ]; then
    OTHER_OPUSFILE_DYLIBS+=(../$OPUSFILE_VERSION-$OTHER_ARCH/$OPUSFILE_DYLIB)
    OTHER_OPUSFILE_STATICLIBS+=(../$OPUSFILE_VERSION-$OTHER_ARCH/$OPUSFILE_STATICLIB)
  fi
done

lipo -create ./$OPUSFILE_DYLIB ${OTHER_OPUSFILE_DYLIBS[@]} -output ./$OPUSFILE_DYLIB
lipo -create ./$OPUSFILE_STATICLIB ${OTHER_OPUSFILE_STATICLIBS[@]} -output ./$OPUSFILE_STATICLIB
make install
cd ..
