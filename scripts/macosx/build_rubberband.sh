#!/bin/bash

# Echo commands
set -x

# Quit on errors
set -e

# Get the path to our scripts folder.
pushd `dirname $0` > /dev/null
PROGDIR=`pwd -P`
popd > /dev/null

export VERSION_NUMBER=1.9.0
export VERSION=rubberband-${VERSION_NUMBER}
export ARCHIVE=$VERSION.tar.bz2

echo "Building $VERSION for $MIXXX_ENVIRONMENT_NAME for architectures: ${MIXXX_ARCHS[@]}"

# You may need to change these from version to version.
export DYLIB=lib/librubberband.dylib
export STATICLIB=lib/librubberband.a

for ARCH in ${MIXXX_ARCHS[@]}
do
  mkdir -p $VERSION-$ARCH
  tar -jxf $DEPENDENCIES/$ARCHIVE -C $VERSION-$ARCH --strip-components 1
  cd $VERSION-$ARCH
  source $PROGDIR/environment.sh $ARCH
  export CFLAGS="$CFLAGS -F$OSX_SDK/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/"
  export CXXFLAGS="$CXXFLAGS -F$OSX_SDK/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/"
  export LDFLAGS="$LDFLAGS -framework Accelerate -F$OSX_SDK/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/"
  SRC_CFLAGS="-I." SRC_LIBS=" " SNDFILE_CFLAGS=" " SNDFILE_LIBS=" " FFTW_CFLAGS=" " FFTW_LIBS=" " Vamp_LIBS=" " Vamp_CFLAGS=" " ./configure --host $HOST --target $TARGET --prefix=$MIXXX_PREFIX
  # Hack up the Makefile to build on OS X and use Speex / VDSP.
  sed -i -e 's/LIBRARY_INCLUDES := \\/LIBRARY_INCLUDES := src\/speex\/speex_resampler.h \\/g' Makefile
  sed -i -e 's/LIBRARY_SOURCES := \\/LIBRARY_SOURCES := src\/speex\/resample.c \\/g' Makefile
  sed -i -e 's/\.so/\.dylib/g' Makefile
  sed -i -e 's/-Wl,-Bsymbolic //g' Makefile
  sed -i -e 's/-shared.*$/-dynamiclib/g' Makefile
  sed -i -e 's/-DHAVE_LIBSAMPLERATE -DHAVE_FFTW3 -DFFTW_DOUBLE_ONLY /-DHAVE_VDSP -DUSE_SPEEX -DMALLOC_IS_ALIGNED /g' Makefile
  make lib
  make static
  make dynamic
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

# We can't make install because the Makefile insists on creating the VAMP plugin
# and the LADSPA plugin and the binary.
export RUBBERBAND_INCLUDE=${MIXXX_PREFIX}/include/rubberband
export RUBBERBAND_LIB=${MIXXX_PREFIX}/lib
mkdir -p $RUBBERBAND_INCLUDE
mkdir -p $RUBBERBAND_LIB
cp rubberband/rubberband-c.h rubberband/RubberBandStretcher.h $RUBBERBAND_INCLUDE
cp lib/librubberband.a lib/librubberband.dylib $RUBBERBAND_LIB
install_name_tool -id $RUBBERBAND_LIB/librubberband.dylib $RUBBERBAND_LIB/librubberband.dylib

cd ..
