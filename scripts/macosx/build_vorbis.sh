#!/bin/bash

# Echo commands
set -x

# Quit on errors
set -e

# Get the path to our scripts folder.
pushd `dirname $0` > /dev/null
PROGDIR=`pwd -P`
popd > /dev/null

export VERSION_NUMBER=1.3.4
export VERSION=libvorbis-${VERSION_NUMBER}
export ARCHIVE=$VERSION.tar.xz

echo "Building $VERSION for $MIXXX_ENVIRONMENT_NAME for architectures: ${MIXXX_ARCHS[@]}"

# You may need to change these from version to version.
export VORBIS_DYLIB=lib/.libs/libvorbis.0.dylib
export VORBIS_STATICLIB=lib/.libs/libvorbis.a
export VORBISENC_DYLIB=lib/.libs/libvorbisenc.2.dylib
export VORBISENC_STATICLIB=lib/.libs/libvorbisenc.a
export VORBISFILE_DYLIB=lib/.libs/libvorbisfile.3.dylib
export VORBISFILE_STATICLIB=lib/.libs/libvorbisfile.a

for ARCH in ${MIXXX_ARCHS[@]}
do
  mkdir -p $VERSION-$ARCH
  tar -zxf $DEPENDENCIES/$ARCHIVE -C $VERSION-$ARCH --strip-components 1
  cd $VERSION-$ARCH
  source $PROGDIR/environment.sh $ARCH
  ./configure --host $HOST --target $TARGET --disable-dependency-tracking --prefix=$MIXXX_PREFIX
  make
  cd ..
done

# Install the i386 version in case there are binaries we want to run (our host is i386)
export ARCH=i386
cd $VERSION-$ARCH
source $PROGDIR/environment.sh $ARCH
export CC="$CC $CFLAGS"
export CXX="$CXX $CXXFLAGS"

OTHER_VORBIS_DYLIBS=()
OTHER_VORBIS_STATICLIBS=()
OTHER_VORBISENC_DYLIBS=()
OTHER_VORBISENC_STATICLIBS=()
OTHER_VORBISFILE_DYLIBS=()
OTHER_VORBISFILE_STATICLIBS=()
for OTHER_ARCH in ${MIXXX_ARCHS[@]}
do
  if [ $OTHER_ARCH != $ARCH ]; then
    OTHER_VORBIS_DYLIBS+=(../$VERSION-$OTHER_ARCH/$VORBIS_DYLIB)
    OTHER_VORBIS_STATICLIBS+=(../$VERSION-$OTHER_ARCH/$VORBIS_STATICLIB)
    OTHER_VORBISENC_DYLIBS+=(../$VERSION-$OTHER_ARCH/$VORBISENC_DYLIB)
    OTHER_VORBISENC_STATICLIBS+=(../$VERSION-$OTHER_ARCH/$VORBISENC_STATICLIB)
    OTHER_VORBISFILE_DYLIBS+=(../$VERSION-$OTHER_ARCH/$VORBISFILE_DYLIB)
    OTHER_VORBISFILE_STATICLIBS+=(../$VERSION-$OTHER_ARCH/$VORBISFILE_STATICLIB)
  fi
done

lipo -create ./$VORBIS_DYLIB ${OTHER_VORBIS_DYLIBS[@]} -output ./$VORBIS_DYLIB
lipo -create ./$VORBIS_STATICLIB ${OTHER_VORBIS_STATICLIBS[@]} -output ./$VORBIS_STATICLIB
lipo -create ./$VORBISENC_DYLIB ${OTHER_VORBISENC_DYLIBS[@]} -output ./$VORBISENC_DYLIB
lipo -create ./$VORBISENC_STATICLIB ${OTHER_VORBISENC_STATICLIBS[@]} -output ./$VORBISENC_STATICLIB
lipo -create ./$VORBISFILE_DYLIB ${OTHER_VORBISFILE_DYLIBS[@]} -output ./$VORBISFILE_DYLIB
lipo -create ./$VORBISFILE_STATICLIB ${OTHER_VORBISFILE_STATICLIBS[@]} -output ./$VORBISFILE_STATICLIB
make install
cd ..
