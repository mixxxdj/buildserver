#!/bin/bash

# Echo commands
set -x

# Quit on errors
set -e

# Get the path to our scripts folder.
pushd `dirname $0` > /dev/null
PROGDIR=`pwd -P`
popd > /dev/null

export VERSION_NUMBER=4.0
export VERSION=ccache-${VERSION_NUMBER}
export ARCHIVE=$VERSION.tar.gz

echo "Building $VERSION for $MIXXX_ENVIRONMENT_NAME for architectures: ${MIXXX_ARCHS[@]}"

for ARCH in ${MIXXX_ARCHS[@]}
do
  mkdir -p $VERSION-$ARCH
  tar -zxf $DEPENDENCIES/$ARCHIVE -C $VERSION-$ARCH --strip-components 1
  cd $VERSION-$ARCH
  source $PROGDIR/environment.sh $ARCH
  # environment.sh sets CC/CXX to use ccache, but it hasn't been built yet at this point
  export XCODE_ROOT=$(xcode-select -print-path)
  export CC="${XCODE_ROOT}/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang"
  export CXX="${XCODE_ROOT}/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++"
  export CPP="$CC -E"
  export CXXCPP="$CXX -E"
  cmake . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$MIXXX_PREFIX" -DCMAKE_OSX_DEPLOYMENT_TARGET="$MACOSX_DEPLOYMENT_TARGET" -DCMAKE_OSX_SYSROOT="$SDKROOT" -DCMAKE_VERBOSE_MAKEFILE=TRUE -DBUILD_SHARED_LIBS=TRUE
  cmake --build . --target install
  cd ..
done

ccache -M 5G
