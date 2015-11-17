#!/bin/bash

# Echo commands
set -x

# Quit on errors
set -e

# Get the path to our scripts folder.
pushd `dirname $0` > /dev/null
PROGDIR=`pwd -P`
popd > /dev/null

export VERSION_NUMBER=2.5.0
export VERSION=protobuf-${VERSION_NUMBER}
export ARCHIVE=$VERSION.tar.gz

echo "Building $VERSION for $MIXXX_ENVIRONMENT_NAME for architectures: ${MIXXX_ARCHS[@]}"

# You may need to change these from version to version.
export PROTOBUF_DYLIB=src/.libs/libprotobuf.8.dylib
export PROTOBUF_STATICLIB=src/.libs/libprotobuf.a
export PROTOBUF_LITE_DYLIB=src/.libs/libprotobuf-lite.8.dylib
export PROTOBUF_LITE_STATICLIB=src/.libs/libprotobuf-lite.a
export PROTOC_DYLIB=src/.libs/libprotoc.8.dylib
export PROTOC_STATICLIB=src/.libs/libprotoc.a

# We need a working version of protoc for this architecture (regardless of
# whether we are building for $HOST_ARCH).
mkdir -p $VERSION-$HOST_ARCH
tar -zxf $DEPENDENCIES/$ARCHIVE -C $VERSION-$HOST_ARCH --strip-components 1
cd $VERSION-$HOST_ARCH
source $PROGDIR/environment.sh $HOST_ARCH
COMMON_FLAGS="--host $HOST --target $TARGET --disable-dependency-tracking --prefix=$MIXXX_PREFIX"
./configure $COMMON_FLAGS
make
make install
cd ..

# Now build all architectures that are not $HOST_ARCH.
for ARCH in ${MIXXX_ARCHS[@]}
do
  if [[ "$ARCH" != "$HOST_ARCH" ]]; then
    mkdir -p $VERSION-$ARCH
    tar -zxf $DEPENDENCIES/$ARCHIVE -C $VERSION-$ARCH --strip-components 1
    cd $VERSION-$ARCH
    source $PROGDIR/environment.sh $ARCH
    COMMON_FLAGS="--host $HOST --target $TARGET --disable-dependency-tracking --prefix=$MIXXX_PREFIX"
    # We built for $HOST_ARCH above so we already have a working protoc.
    ./configure $COMMON_FLAGS --with-protoc=../$VERSION-$HOST_ARCH/src/protoc
    make
    cd ..
  fi
done

# We already installed the $HOST_ARCH version so now build the library version
# we need. This may or may not include HOST_ARCH.
MIXXX_ARCHS_ARRAY=($MIXXX_ARCHS)
export ARCH=${MIXXX_ARCHS_ARRAY[0]}
cd $VERSION-$ARCH
source $PROGDIR/environment.sh $ARCH

OTHER_PROTOBUF_DYLIBS=()
OTHER_PROTOBUF_STATICLIBS=()
OTHER_PROTOBUF_LITE_DYLIBS=()
OTHER_PROTOBUF_LITE_STATICLIBS=()
OTHER_PROTOC_DYLIBS=()
OTHER_PROTOC_STATICLIBS=()
for OTHER_ARCH in ${MIXXX_ARCHS[@]}
do
  if [[ "$OTHER_ARCH" != "$ARCH" ]]; then
    OTHER_PROTOBUF_DYLIBS+=(../$VERSION-$OTHER_ARCH/$PROTOBUF_DYLIB)
    OTHER_PROTOBUF_STATICLIBS+=(../$VERSION-$OTHER_ARCH/$PROTOBUF_STATICLIB)
    OTHER_PROTOBUF_LITE_DYLIBS+=(../$VERSION-$OTHER_ARCH/$PROTOBUF_LITE_DYLIB)
    OTHER_PROTOBUF_LITE_STATICLIBS+=(../$VERSION-$OTHER_ARCH/$PROTOBUF_LITE_STATICLIB)
    OTHER_PROTOC_DYLIBS+=(../$VERSION-$OTHER_ARCH/$PROTOC_DYLIB)
    OTHER_PROTOC_STATICLIBS+=(../$VERSION-$OTHER_ARCH/$PROTOC_STATICLIB)
  fi
done

lipo -create ./$PROTOBUF_DYLIB ${OTHER_PROTOBUF_DYLIBS[@]} -output ./$PROTOBUF_DYLIB
lipo -create ./$PROTOBUF_STATICLIB ${OTHER_PROTOBUF_STATICLIBS[@]} -output ./$PROTOBUF_STATICLIB
lipo -create ./$PROTOBUF_LITE_DYLIB ${OTHER_PROTOBUF_LITE_DYLIBS[@]} -output ./$PROTOBUF_LITE_DYLIB
lipo -create ./$PROTOBUF_LITE_STATICLIB ${OTHER_PROTOBUF_LITE_STATICLIBS[@]} -output ./$PROTOBUF_LITE_STATICLIB
lipo -create ./$PROTOC_DYLIB ${OTHER_PROTOC_DYLIBS[@]} -output ./$PROTOC_DYLIB
lipo -create ./$PROTOC_STATICLIB ${OTHER_PROTOC_STATICLIBS[@]} -output ./$PROTOC_STATICLIB

make install
ln -s $MIXXX_PREFIX/bin/$HOST-protoc $MIXXX_PREFIX/bin/protoc

cd ..
