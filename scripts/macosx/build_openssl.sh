#!/bin/bash

# Echo commands
set -x

# Quit on errors
set -e

# Get the path to our scripts folder.
pushd `dirname $0` > /dev/null
PROGDIR=`pwd -P`
popd > /dev/null

export VERSION_NUMBER=1.0.2h
export VERSION=openssl-${VERSION_NUMBER}
export ARCHIVE=$VERSION.tar.gz

echo "Building $VERSION for $MIXXX_ENVIRONMENT_NAME for architectures: ${MIXXX_ARCHS[@]}"

# You may need to change these from version to version.
export LIBCRYPTO_DYLIB=libcrypto.1.0.0.dylib
export LIBCRYPTO_STATICLIB=libcrypto.a
export LIBSSL_DYLIB=libssl.1.0.0.dylib
export LIBSSL_STATICLIB=libssl.a

for ARCH in ${MIXXX_ARCHS[@]}
do
  mkdir -p $VERSION-$ARCH
  tar -zxf $DEPENDENCIES/$ARCHIVE -C $VERSION-$ARCH --strip-components 1
  cd $VERSION-$ARCH
  source $PROGDIR/environment.sh $ARCH
  ./Configure --prefix=$MIXXX_PREFIX --shared darwin64-$ARCH-cc 
  make
  cd ..
done

# Install the $HOST_ARCH version in case there are binaries we want to run.
export ARCH=$HOST_ARCH
cd $VERSION-$ARCH
source $PROGDIR/environment.sh $ARCH

OTHER_LIBCRYPTO_DYLIBS=()
OTHER_LIBCRYPTO_STATICLIBS=()
OTHER_LIBSSL_DYLIBS=()
OTHER_LIBSSL_STATICLIBS=()
for OTHER_ARCH in ${MIXXX_ARCHS[@]}
do
  if [ $OTHER_ARCH != $ARCH ]; then
    OTHER_LIBCRYPTO_DYLIBS+=(../$VERSION-$OTHER_ARCH/$LIBCRYPTO_DYLIB)
    OTHER_LIBCRYPTO_STATICLIBS+=(../$VERSION-$OTHER_ARCH/$LIBCRYPTO_STATICLIB)
    OTHER_LIBSSL_DYLIBS+=(../$VERSION-$OTHER_ARCH/$LIBSSL_DYLIB)
    OTHER_LIBSSL_STATICLIBS+=(../$VERSION-$OTHER_ARCH/$LIBSSL_STATICLIB)
  fi
done

lipo -create ./$LIBCRYPTO_DYLIB ${OTHER_LIBCRYPTO_DYLIBS[@]} -output ./$LIBCRYPTO_DYLIB
lipo -create ./$LIBCRYPTO_STATICLIB ${OTHER_LIBCRYPTO_STATICLIBS[@]} -output ./$LIBCRYPTO_STATICLIB
lipo -create ./$LIBSSL_DYLIB ${OTHER_LIBSSL_DYLIBS[@]} -output ./$LIBSSL_DYLIB
lipo -create ./$LIBSSL_STATICLIB ${OTHER_LIBSSL_STATICLIBS[@]} -output ./$LIBSSL_STATICLIB
make install
cd ..
