#!/bin/bash

# Quit on errors
set -e

# Get the path to our scripts folder.
pushd `dirname $0` > /dev/null
PROGDIR=`pwd -P`
popd > /dev/null

usage() { echo "Usage: $0 --name <name> [--macosx-sdk <version>] [--maxosx-target <version>] [--enable-ppc] [--enable-i386] [--enable-x86-64]" 1>&2; exit 1; }

MIXXX_ENVIRONMENT_NAME=""
MIXXX_MACOSX_SDK='10.9'
MIXXX_MACOSX_TARGET='10.9'
ENABLE_I386=false
ENABLE_X86_64=false
ENABLE_PPC=false
while getopts ":-:" o; do
    case "${o}" in
        -)
            case "${OPTARG}" in
                name)
                    MIXXX_ENVIRONMENT_NAME="${!OPTIND}"; OPTIND=$(( $OPTIND + 1 ))
                    ;;
                macosx-sdk)
                    MIXXX_MACOSX_SDK="${!OPTIND}"; OPTIND=$(( $OPTIND + 1 ))
                    ;;
                macosx-target)
                    MIXXX_MACOSX_TARGET="${!OPTIND}"; OPTIND=$(( $OPTIND + 1 ))
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

export MIXXX_PREFIX=`pwd -P`/environment/$MIXXX_ENVIRONMENT_NAME
echo "Building environment ${MIXXX_ENVIRONMENT_NAME} for architectures ${ARCHS[@]} in $MIXXX_PREFIX on Mac OS X (sdk: $MIXXX_MACOSX_SDK, target: $MIXXX_MACOSX_TARGET)"

export DEPENDENCIES=`pwd -P`/dependencies
if [[ ! -d $DEPENDENCIES ]]; then
    echo "There must be a 'dependencies' folder in the current directory with archives of all the dependencies."
    exit 1
fi

mkdir -p build/$MIXXX_ENVIRONMENT_NAME
mkdir -p environment/$MIXXX_ENVIRONMENT_NAME
cd build/$MIXXX_ENVIRONMENT_NAME

# Set this to the appropriate host installation type.
source $PROGDIR/environment.sh
export HOST=$TARGET_X86_64
export HOST_ARCH=x86_64

# Setup cmake first so that we can build other projects that use cmake.
$PROGDIR/build_cmake.sh

$PROGDIR/build_chromaprint.sh
$PROGDIR/build_flac.sh
$PROGDIR/build_hss1394.sh
$PROGDIR/build_ogg.sh
$PROGDIR/build_opus.sh # depends on ogg
$PROGDIR/build_portaudio.sh
$PROGDIR/build_portmidi.sh
$PROGDIR/build_protobuf.sh
$PROGDIR/build_rubberband.sh
$PROGDIR/build_sqlite.sh
$PROGDIR/build_qt4.sh # depends on sqlite
$PROGDIR/build_sndfile.sh
$PROGDIR/build_taglib.sh
$PROGDIR/build_vorbis.sh

# Shout depends on libogg and libvorbis.
$PROGDIR/build_shout.sh
