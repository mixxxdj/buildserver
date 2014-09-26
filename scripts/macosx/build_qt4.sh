#!/bin/bash

# Echo commands
set -x

# Quit on errors
set -e

# Get the path to our scripts folder.
pushd `dirname $0` > /dev/null
PROGDIR=`pwd -P`
popd > /dev/null

export VERSION_NUMBER=4.8.6
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

# Apply patch from QTBUG-23258 to fix Qt 4.8.2 and 4.8.3 build on OS X 10.5 SDK
#curl https://bugreports.qt-project.org/secure/attachment/26712/Patch-Qt-4.8-for-10.5 > Patch-Qt-4.8-for-10.5
#patch -p1 < Patch-Qt-4.8-for-10.5

# Qt uses -arch x86 not -arch i386
# -arch ppc is no longer supported.

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
# Mixxx may want to call sqlite functions directly so we use the statically
# linked version of SQLite (-qt-sql-sqlite) and link it to the system SQLite
# (-system-sqlite) instead of the SQLite plugin
# (-plugin-sql-sqlite).
# See:
# - http://www.mimec.org/node/296
./configure -opensource -prefix $QTDIR ${QT_ARCHS[@]} -sdk $OSX_SDK -system-sqlite -qt-sql-sqlite -no-phonon -no-webkit -platform macx-g++ -no-qt3support -debug-and-release -nomake examples -nomake demos -confirm-license

# NOTE: Using -ffast-math will fail when building SQLite so either remove -ffast-math from environment.sh temporarily or remove -ffast-math from the SQLite Makefiles (you'll have to do it for QtWebkit and QtSql). You can do this as the build fails since it will complain about -ffast-math. We remove it with sed:
find src/sql -name 'Makefile*' -exec sed -i -e 's/-ffast-math //g' "{}" \;
find src/plugins/sqldrivers/sqlite -name 'Makefile*' -exec sed -i -e 's/-ffast-math //g' "{}" \;
find src/plugins/sqldrivers/sqlite2 -name 'Makefile*' -exec sed -i -e 's/-ffast-math //g' "{}" \;
# We don't build with webkit (see above comment). Re-enable this if you turn it
# on.
#find src/3rdparty/webkit/Source/WebCore -name 'Makefile*' -exec sed -i -e 's/-ffast-math //g' "{}" \;

make && make install
cd ..
