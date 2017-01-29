#!/bin/bash

# Echo commands
set -x

# Quit on errors
set -e

# Get the path to our scripts folder.
pushd `dirname $0` > /dev/null
PROGDIR=`pwd -P`
popd > /dev/null

export VERSION_NUMBER=20161030
export VERSION=pa_stable_v190600_${VERSION_NUMBER}
export ARCHIVE=$VERSION.tgz

echo "Building $VERSION for $MIXXX_ENVIRONMENT_NAME for architectures: ${MIXXX_ARCHS[@]}"

# You may need to change these from version to version.
export DYLIB=lib/.libs/libportaudio.2.dylib
export STATICLIB=lib/.libs/libportaudio.a

for ARCH in ${MIXXX_ARCHS[@]}
do
  mkdir -p $VERSION-$ARCH
  tar -zxf $DEPENDENCIES/$ARCHIVE -C $VERSION-$ARCH --strip-components 1
  cd $VERSION-$ARCH
  source $PROGDIR/environment.sh $ARCH

  # As of the PA 2011/3/26 snapshot, a deprecated API function of CoreAudio is
  # used which blocks the build due to -Werror. -Wno-deprecated-declarations
  # allows these errors to pass.
  export CFLAGS="$CFLAGS -Wno-deprecated-declarations"
  export CXXFLAGS=$CFLAGS

  # Mac universal in this case includes ppc64 which we aren't supporting.
  ./configure --host $HOST --target $TARGET --prefix=$MIXXX_PREFIX --disable-mac-universal
  make
  cd ..
done

# Install the $HOST_ARCH version in case there are binaries we want to run.
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
