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

source $PROGDIR/environment.sh
export QTDIR=$MIXXX_PREFIX/Qt-${VERSION_NUMBER}

QT_ARCHS=()
for ARCH in ${MIXXX_ARCHS[@]}
do
  if [[ "$ARCH" == "i386" ]]; then
      QT_ARCHS+=(-arch x86)
  elif [[ "$ARCH" == "x86_64" ]]; then
      QT_ARCHS+=(-arch x86_64)
  elif [[ "$ARCH" == "ppc" ]]; then
      QT_ARCHS+=(-arch ppc)
  fi
done

echo "Building $VERSION for $MIXXX_ENVIRONMENT_NAME for architectures: ${QT_ARCHS[@]}"

# Mixxx does not use Phonon or Webkit and as of Qt 4.8.6 Phonon does not build
# with i386/x86_64 using gcc-llvm (the default GCC included with XCode).
# See:
# - http://llvm.org/bugs/show_bug.cgi?id=16805
# - https://qt.gitorious.org/qt/qt/source/5267ff3c462986f514f33998a2614b8f9c22402e:src/plugins/phonon/qt7/qt7.pro#L5
./configure -opensource -prefix $QTDIR ${QT_ARCHS[@]} -sdk macosx${MIXXX_MACOSX_SDK} -plugin-sql-sqlite -platform macx-g++ -release -confirm-license -force-debug-info -nomake examples -nomake tests -skip qt3d -skip qtwebengine

# NOTE: Using -ffast-math will fail when building SQLite so either remove -ffast-math from environment.sh temporarily or remove -ffast-math from the SQLite Makefiles. You can do this as the build fails since it will complain about -ffast-math. We remove it with sed:
find qtbase/src/plugins/sqldrivers/sqlite -name 'Makefile*' -exec sed -i -e 's/-ffast-math //g' "{}" \;

make && make install
cd ..
