#!/bin/bash

# Quit on errors
set -e

# Get the path to our scripts folder.
pushd `dirname $0` > /dev/null
PROGDIR=`pwd -P`
popd > /dev/null

usage() { echo "Usage: $0 --name <name> [--dependency-cache <path>] [--macosx-sdk <version>] [--maxosx-target <version>] [--macosx-stdlib <stdlib>] [--enable-ppc] [--enable-i386] [--enable-x86-64]" 1>&2; exit 1; }

MIXXX_ENVIRONMENT_NAME=""
# Use the default installed macOS SDK, allowing override via commandline flag.
MIXXX_MACOSX_SDK=$(xcodebuild -version -sdk macosx SDKVersion)
# Qt 5.12 requires a minimum of macOS 10.12.
# https://doc.qt.io/qt-5.12/supported-platforms.html
MIXXX_MACOSX_TARGET='10.12'
MIXXX_MACOSX_STDLIB='libc++'
MIXXX_QT_VERSION='5.12.10'
ENABLE_I386=false
ENABLE_X86_64=false
ENABLE_PPC=false
DEPENDENCIES=`pwd -P`/dependencies
while getopts ":-:" o; do
    case "${o}" in
        -)
            case "${OPTARG}" in
                name)
                    MIXXX_ENVIRONMENT_NAME="${!OPTIND}"; OPTIND=$(( $OPTIND + 1 ))
                    ;;
                dependency-cache)
                    DEPENDENCIES="${!OPTIND}"; OPTIND=$(( $OPTIND + 1 ))
                    ;;
                macosx-sdk)
                    MIXXX_MACOSX_SDK="${!OPTIND}"; OPTIND=$(( $OPTIND + 1 ))
                    ;;
                macosx-target)
                    MIXXX_MACOSX_TARGET="${!OPTIND}"; OPTIND=$(( $OPTIND + 1 ))
                    ;;
                macosx-stdlib)
                    MIXXX_MACOSX_STDLIB="${!OPTIND}"; OPTIND=$(( $OPTIND + 1 ))
                    ;;
                enable-ppc)
                    ENABLE_PPC=true
                    ;;
                enable-i386)
                    ENABLE_I386=true
                    ;;
                enable-x86-64)
                    ENABLE_X86_64=true
                    ;;
            esac;;
        *)
            echo "Usage: ${o}"
            ;;
    esac
done
shift $((OPTIND-1))

if [ -z "${MIXXX_ENVIRONMENT_NAME}" ]; then
    usage
fi

export MIXXX_ENVIRONMENT_NAME
export MIXXX_MACOSX_SDK
export MIXXX_MACOSX_TARGET
export MIXXX_MACOSX_STDLIB
export MIXXX_QT_VERSION
ARCHS=()
if $ENABLE_I386; then
    ARCHS+=(i386)
fi
if $ENABLE_X86_64; then
    ARCHS+=(x86_64)
fi
if $ENABLE_PPC; then
    ARCHS+=(ppc)
fi
export MIXXX_ARCHS="${ARCHS[@]}"

if [ ${#ARCHS[@]} -eq 0 ]; then
    echo "You must select an architecture."
    usage
    exit 1
fi

export DEPENDENCIES
if [[ ! -d $DEPENDENCIES ]]; then
    echo "The dependency cache (${DEPENDENCIES}) does not exist. Have you run download_dependencies.sh?"
    exit 1
fi

export MIXXX_PREFIX=`pwd -P`/environment/$MIXXX_ENVIRONMENT_NAME
echo "Building environment ${MIXXX_ENVIRONMENT_NAME} for architectures ${ARCHS[@]} in $MIXXX_PREFIX on Mac OS X (sdk: $MIXXX_MACOSX_SDK, target: $MIXXX_MACOSX_TARGET, stdlib: $MIXXX_MACOSX_STDLIB)"

mkdir -p build/$MIXXX_ENVIRONMENT_NAME
mkdir -p environment/$MIXXX_ENVIRONMENT_NAME
cd build/$MIXXX_ENVIRONMENT_NAME

# Set this to the appropriate host installation type.
source $PROGDIR/environment.sh
export HOST=$TARGET_X86_64
export HOST_ARCH=x86_64

# Setup build systems first so that we can build other projects that use them.
# $PROGDIR/build_cmake.sh
# $PROGDIR/build_zstd.sh
# $PROGDIR/build_ccache.sh # depends on zstd
# $PROGDIR/build_scons.sh
# $PROGDIR/build_autoconf.sh
# $PROGDIR/build_automake.sh
# $PROGDIR/build_libtool.sh
# $PROGDIR/build_pkgconfig.sh
# 
# $PROGDIR/build_chromaprint.sh
# $PROGDIR/build_flac.sh
# $PROGDIR/build_hss1394.sh
# $PROGDIR/build_libusb.sh
# $PROGDIR/build_ogg.sh
# $PROGDIR/build_opus.sh # depends on ogg
# $PROGDIR/build_portaudio.sh
# $PROGDIR/build_portmidi.sh
# $PROGDIR/build_protobuf.sh
# $PROGDIR/build_rubberband.sh
# $PROGDIR/build_sqlite.sh
# $PROGDIR/build_openssl.sh
# $PROGDIR/build_sndfile.sh
# $PROGDIR/build_taglib.sh
# $PROGDIR/build_vorbis.sh
# # Shout & its IDJC variant depend on openssl, libogg and libvorbis.
# $PROGDIR/build_shout.sh
# $PROGDIR/build_shoutidjc.sh
# $PROGDIR/build_lame.sh
# $PROGDIR/build_lv2.sh
# $PROGDIR/build_serd.sh
# $PROGDIR/build_sord.sh  # depends on serd
# $PROGDIR/build_sratom.sh  # depends on lv2, serd, sord
# $PROGDIR/build_lilv.sh  # depends on lv2, serd, sord, sratom
# 
# # Build Qt last so we catch errors in the above dependencies faster.
# $PROGDIR/build_qt5.sh # depends on sqlite
# $PROGDIR/build_qtkeychain.sh  # depends on qt5

# After installing everything to the environment, make all the libraries and
# executables relocatable.
cd ../..
$PROGDIR/relocate_environment.py environment/$MIXXX_ENVIRONMENT_NAME
