#!/usr/bin/env bash

set -e

if [ "$1" = "" ]; then
    DEPENDENCIES=dependencies
else
    DEPENDENCIES=$1
fi
echo "Downloading dependencies to ${DEPENDENCIES}."
mkdir -p $DEPENDENCIES

function download_and_verify {
  URL=$1
  REMOTE_FILENAME=$(basename $URL)
  EXPECTED_SHA=$2
  FILENAME=$3
  if [[ "$FILENAME" = "" ]]; then
    FILENAME=$REMOTE_FILENAME
  fi

  if [[ -e "$DEPENDENCIES/$FILENAME" ]]; then
      echo "$FILENAME exists. Not re-downloading."
  else
      curl -s -L -o $DEPENDENCIES/$FILENAME $URL
  fi

  SHA=$(shasum -a 256 -b $DEPENDENCIES/$FILENAME | cut -d' ' -f1)
  if [[ "$EXPECTED_SHA" = "$SHA" ]]; then
      echo "$FILENAME matches expected sha256sum."
  else
      echo "$FILENAME did not match expected sha256sum. Expected: $EXPECTED_SHA Observed: $SHA"
      exit 1
  fi
}

download_and_verify http://ftp.gnu.org/gnu/autoconf/autoconf-2.69.tar.xz 64ebcec9f8ac5b2487125a86a7760d2591ac9e1d3dbd59489633f9de62a57684
download_and_verify http://ftp.gnu.org/gnu/automake/automake-1.16.2.tar.xz ccc459de3d710e066ab9e12d2f119bd164a08c9341ca24ba22c9adaa179eedd0
download_and_verify http://ftp.gnu.org/gnu/libtool/libtool-2.4.6.tar.xz 7c87a8c2c8c0fc9cd5019e402bed4292462d00a718a7cd5f11218153bf28b26f
download_and_verify https://pkg-config.freedesktop.org/releases/pkg-config-0.29.2.tar.gz 6fc69c01688c9458a57eb9a1664c9aba372ccda420a02bf4429fe610e7e7d591
download_and_verify https://github.com/acoustid/chromaprint/releases/download/v1.5.0/chromaprint-1.5.0.tar.gz 573a5400e635b3823fc2394cfa7a217fbb46e8e50ecebd4a61991451a8af766a
download_and_verify https://github.com/ccache/ccache/releases/download/v4.0/ccache-4.0.tar.gz ac97af86679028ebc8555c99318352588ff50f515fc3a7f8ed21a8ad367e3d45
download_and_verify https://cmake.org/files/v3.19/cmake-3.19.2.tar.gz e3e0fd3b23b7fb13e1a856581078e0776ffa2df4e9d3164039c36d3315e0c7f0
download_and_verify http://downloads.xiph.org/releases/flac/flac-1.3.3.tar.xz 213e82bd716c9de6db2f98bcadbc4c24c7e2efe8c75939a1a84e28539c4e1748
download_and_verify http://fftw.org/fftw-3.3.9.tar.gz bf2c7ce40b04ae811af714deb512510cc2c17b9ab9d6ddcf49fe4487eea7af3d
download_and_verify https://github.com/mixxxdj/libKeyFinder/archive/v2.2.3.tar.gz 6d8c1306ef9fa31f434ebb81ae2b9df62a001b234edbbf32a727109f8f251c3f libKeyFinder-2.2.3.tar.gz
download_and_verify http://downloads.mixxx.org/hss1394-r8.tar.gz d5c87a92ad8e277b418300fbbfcc06a0826dfde71f7b10643ac59703d795c002
download_and_verify https://github.com/libusb/libusb/releases/download/v1.0.23/libusb-1.0.23.tar.bz2 db11c06e958a82dac52cf3c65cb4dd2c3f339c8a988665110e0d24d19312ad8d
download_and_verify https://www.openssl.org/source/openssl-1.0.2u.tar.gz ecd0c6ffb493dd06707d38b14bb4d8c2288bb7033735606569d8f90f89669d16
download_and_verify http://downloads.xiph.org/releases/ogg/libogg-1.3.3.tar.xz 4f3fc6178a533d392064f14776b23c397ed4b9f48f5de297aba73b643f955c08
download_and_verify https://ftp.osuosl.org/pub/xiph/releases/opus/opus-1.3.1.tar.gz 65b58e1e25b2a114157014736a3d9dfeaad8d41be1c8179866f144a2fb44ff9d
download_and_verify https://ftp.osuosl.org/pub/xiph/releases/opus/opusfile-0.12.tar.gz 118d8601c12dd6a44f52423e68ca9083cc9f2bfe72da7a8c1acb22a80ae3550b
download_and_verify http://www.portaudio.com/archives/pa_stable_v190600_20161030.tgz f5a21d7dcd6ee84397446fa1fa1a0675bb2e8a4a6dceb4305a8404698d8d1513
download_and_verify http://downloads.sourceforge.net/project/portmedia/portmidi/217/portmidi-src-217.zip 08e9a892bd80bdb1115213fb72dc29a7bf2ff108b378180586aa65f3cfd42e0f
download_and_verify https://github.com/google/protobuf/archive/v2.6.1.tar.gz 2667b7cda4a6bc8a09e5463adf3b5984e08d94e72338277affa8594d8b6e5cd1 protobuf-2.6.1.tar.gz
#download_and_verify https://download.qt.io/official_releases/qt/4.8/4.8.7/qt-everywhere-opensource-src-4.8.7.tar.gz e2882295097e47fe089f8ac741a95fef47e0a73a3f3cdf21b56990638f626ea0
download_and_verify https://download.qt.io/official_releases/qt/5.12/5.12.3/single/qt-everywhere-src-5.12.3.tar.xz 6462ac74c00ff466487d8ef8d0922971aa5b1d5b33c0753308ec9d57711f5a42
download_and_verify https://breakfastquay.com/files/releases/rubberband-1.9.0.tar.bz2 4f5b9509364ea876b4052fc390c079a3ad4ab63a2683aad09662fb905c2dc026
download_and_verify http://downloads.xiph.org/releases/libshout/libshout-2.4.4.tar.gz 8ce90c5d05e7ad1da4c12f185837e8a867c22df2d557b0125afaba4b1438e6c3
download_and_verify https://downloads.sourceforge.net/project/libshoutidjc.idjc.p/libshout-idjc-2.4.1.tar.gz 4751c75fc85fc5d10e5b03753b046bcdee39576278bf30565c751816a87facdf
download_and_verify https://github.com/libsndfile/libsndfile/releases/download/v1.0.30/libsndfile-1.0.30.tar.bz2 9df273302c4fa160567f412e10cc4f76666b66281e7ba48370fb544e87e4611a
download_and_verify http://prdownloads.sourceforge.net/scons/scons-2.5.0.tar.gz eb296b47f23c20aec7d87d35cfa386d3508e01d1caa3040ea6f5bbab2292ace9
download_and_verify https://sqlite.org/2020/sqlite-autoconf-3330000.tar.gz 106a2c48c7f75a298a7557bcc0d5f4f454e5b43811cc738b7ca294d6956bbb15
download_and_verify https://taglib.github.io/releases/taglib-1.11.1.tar.gz b6d1a5a610aae6ff39d93de5efd0fdc787aa9e9dc1e7026fa4c961b26563526b
download_and_verify http://downloads.xiph.org/releases/vorbis/libvorbis-1.3.7.tar.xz b33cc4934322bcbf6efcbacf49e3ca01aadbea4114ec9589d1b1e9d20f72954b
download_and_verify https://downloads.sourceforge.net/project/lame/lame/3.100/lame-3.100.tar.gz ddfe36cab873794038ae2c1210557ad34857a4b6bdc515785d1da9e175b1da1e
download_and_verify http://lv2plug.in/spec/lv2-1.18.0.tar.bz2 90a3e5cf8bdca81b49def917e89fd6bba1d5845261642cd54e7888df0320473f
download_and_verify https://download.drobilla.net/serd-0.30.4.tar.bz2 0c95616a6587bee5e728e026190f4acd5ab6e2400e8890d5c2a93031eab01999
download_and_verify https://download.drobilla.net/sord-0.16.4.tar.bz2 b15998f4e7ad958201346009477d6696e90ee5d3e9aff25e7e9be074372690d7
download_and_verify https://download.drobilla.net/sratom-0.6.6.tar.bz2 fb910bf62a5e69f4430bf09653d386fc4de9ff02bfd58635e1d45cbd31481b9d
download_and_verify http://download.drobilla.net/lilv-0.24.10.tar.bz2 d1bba93d6ddacadb5e742fd10ad732727edb743524de229c70cc90ef81ffc594
download_and_verify https://github.com/frankosterfeld/qtkeychain/archive/v0.11.1.tar.gz 77fc6841c1743d9e6bd499989481cd9239c21bc9bf0760d41a4f4068d2f0a49d qtkeychain-0.11.1.tar.gz
download_and_verify https://github.com/facebook/zstd/releases/download/v1.4.5/zstd-1.4.5.tar.gz 98e91c7c6bf162bf90e4e70fdbc41a8188b9fa8de5ad840c401198014406ce9e
