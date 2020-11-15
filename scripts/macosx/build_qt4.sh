#!/bin/bash

# Echo commands
set -x

# Quit on errors
set -e

# Get the path to our scripts folder.
pushd `dirname $0` > /dev/null
PROGDIR=`pwd -P`
popd > /dev/null

export VERSION_NUMBER=4.8.7
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

# Reset CC / CXX since Qt doesn't like us including -stdlib in them:
export CC="${XCODE_ROOT}/usr/bin/gcc"
export CXX="${XCODE_ROOT}/usr/bin/g++"
export CPP="$CC -E"
export CXXCPP="$CXX -E"

# Qt embeds various OSX platform selections in its makefiles and configure
# scripts. Fix them to use $MIXXX_MACOSX_TARGET.
sed -e "s/MACOSX_DEPLOYMENT_TARGET = 10.5/MACOSX_DEPLOYMENT_TARGET = $MIXXX_MACOSX_TARGET/g" -i '' configure
sed -e "s/MACOSX_DEPLOYMENT_TARGET 10.[45]/MACOSX_DEPLOYMENT_TARGET $MIXXX_MACOSX_TARGET/g" -i '' configure
sed -e "s/-mmacosx-version-min=10.[45]/-mmacosx-version-min=$MIXXX_MACOSX_TARGET/g" -i '' configure
sed -e "s/QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.4/QMAKE_MACOSX_DEPLOYMENT_TARGET = $MIXXX_MACOSX_TARGET/g" -i '' mkspecs/common/mac.conf
sed -e "s/-mmacosx-version-min=10.5/-mmacosx-version-min=$MIXXX_MACOSX_TARGET/g" -i '' mkspecs/common/g++-macx.conf

# Build issue with 10.11 SDK.
curl https://raw.githubusercontent.com/Homebrew/patches/480b7142c4e2ae07de6028f672695eb927a34875/qt/el-capitan.patch | patch -p1

# Mixxx may want to call sqlite functions directly so we use the statically
# linked version of SQLite (-qt-sql-sqlite) and link it to the system SQLite
# (-system-sqlite) instead of the SQLite plugin
# (-plugin-sql-sqlite).
# See:
# - http://www.mimec.org/node/296
# NOTE(rryan): Setting -system-sqlite sets -system-zlib. Set -qt-zlib explicitly.
export OPENSSL_LIBS="-L${MIXXX_PREFIX}/lib -lssl -lcrypto"
./configure -opensource -prefix ${QTDIR} ${QT_ARCHS[@]} -sdk $SDKROOT -system-sqlite -qt-sql-sqlite -qt-zlib -no-phonon -no-webkit -no-qt3support -release -nomake examples -nomake demos -confirm-license -openssl -I${MIXXX_PREFIX}/include -L${MIXXX_PREFIX}/lib

make && make install

cd ..
