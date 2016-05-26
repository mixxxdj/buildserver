#!/bin/bash

# Echo commands
set -x

# Quit on errors
set -e

# Get the path to our scripts folder.
pushd `dirname $0` > /dev/null
PROGDIR=`pwd -P`
popd > /dev/null

export VERSION_NUMBER=5.6.0
export VERSION=qt-everywhere-opensource-src-${VERSION_NUMBER}
export ARCHIVE=$VERSION.tar.gz

echo "Building $VERSION for $MIXXX_ENVIRONMENT_NAME for architectures: ${MIXXX_ARCHS[@]}"

tar -zxf $DEPENDENCIES/$ARCHIVE
cd $VERSION
export ARCH=
# Qt gets sad if you use -arch in your CFLAGS/CXXFLAGS. For some reason it does some cutting / munging of your flags and you end up with lone '-arch' flags in your CFLAGS/CXXFLAGS which breaks the build.
export ARCH_FLAGS=

# Don't use -ffast-math when building Qt. It's incompatible with sqlite.
export DISABLE_FFAST_MATH=yes

source $PROGDIR/environment.sh
export QTDIR=$MIXXX_PREFIX/Qt-${VERSION_NUMBER}

# Qt uses -arch x86 not -arch i386. -arch ppc is no longer supported.
QT_ARCHS=()
for ARCH in ${MIXXX_ARCHS[@]}
do
  if [[ "$ARCH" == "i386" ]]; then
      QT_ARCHS+=(-arch x86)
  elif [[ "$ARCH" == "x86_64" ]]; then
      QT_ARCHS+=(-arch x86_64)
  elif [[ "$ARCH" == "ppc" ]]; then
      echo "ppc is unsupported by Qt!"
      exit 1
  fi
done

echo "Building $VERSION for $MIXXX_ENVIRONMENT_NAME for architectures: ${QT_ARCHS[@]}"

# Mixxx may want to call sqlite functions directly so we use the statically
# linked version of SQLite (-qt-sql-sqlite) and link it to the system SQLite
# (-system-sqlite) instead of the SQLite plugin
# (-plugin-sql-sqlite).
# See:
# - http://www.mimec.org/node/296
# NOTE(rryan): Setting -system-sqlite sets -system-zlib. Set -qt-zlib explicitly.
./configure -opensource -prefix $QTDIR ${QT_ARCHS[@]} -sdk macosx${MIXXX_MACOSX_SDK} -system-sqlite -qt-sql-sqlite -qt-zlib -platform macx-g++ -release -confirm-license -force-debug-info -nomake examples -nomake tests -skip qt3d -skip qtwebengine

make && make install
cd ..
