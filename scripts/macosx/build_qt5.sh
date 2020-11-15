#!/bin/bash

# Echo commands
set -x

# Quit on errors
set -e

# Get the path to our scripts folder.
pushd `dirname $0` > /dev/null
PROGDIR=`pwd -P`
popd > /dev/null

export VERSION_NUMBER=${MIXXX_QT_VERSION}
export VERSION=qt-everywhere-src-${VERSION_NUMBER}
export ARCHIVE=$VERSION.tar.xz

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

# Mixxx may want to call sqlite functions directly so we use the statically
# linked version of SQLite (-sql-sqlite) and link it to the system SQLite
# (-system-sqlite) instead of the SQLite plugin
# (-plugin-sql-sqlite).
# See:
# - http://www.mimec.org/node/296
# NOTE(rryan): Setting -system-sqlite sets -system-zlib. Set -qt-zlib explicitly.
# Disable relocatable qt due to https://bugs.launchpad.net/mixxx/+bug/1871238
# TODO(rryan): Disable framework build or use frameworks properly. Qt assumes
# that if frameworks are enabled then they are used for deployment.
./configure -opensource -prefix $QTDIR -sdk macosx${MIXXX_MACOSX_SDK} -ccache -system-sqlite -sql-sqlite -qt-zlib -platform macx-clang -release -confirm-license -securetransport -force-debug-info -nomake examples -nomake tests -skip qt3d -skip qtwebengine -I${MIXXX_PREFIX}/include -L${MIXXX_PREFIX}/lib

make && make install
cd ..
