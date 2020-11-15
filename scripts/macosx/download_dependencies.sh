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
download_and_verify https://pkg-config.freedesktop.org/releases/pkg-config-0.29.1.tar.gz beb43c9e064555469bd4390dcfd8030b1536e0aa103f08d7abf7ae8cac0cb001
download_and_verify http://ftp.gnu.org/gnu/automake/automake-1.16.2.tar.xz ccc459de3d710e066ab9e12d2f119bd164a08c9341ca24ba22c9adaa179eedd0
download_and_verify http://ftp.gnu.org/gnu/libtool/libtool-2.4.6.tar.xz 7c87a8c2c8c0fc9cd5019e402bed4292462d00a718a7cd5f11218153bf28b26f
download_and_verify https://github.com/acoustid/chromaprint/releases/download/v1.5.0/chromaprint-1.5.0.tar.gz 573a5400e635b3823fc2394cfa7a217fbb46e8e50ecebd4a61991451a8af766a
download_and_verify https://cmake.org/files/v3.18/cmake-3.18.4.tar.gz 597c61358e6a92ecbfad42a9b5321ddd801fc7e7eca08441307c9138382d4f77
download_and_verify http://downloads.xiph.org/releases/flac/flac-1.3.3.tar.xz 213e82bd716c9de6db2f98bcadbc4c24c7e2efe8c75939a1a84e28539c4e1748
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
download_and_verify https://download.qt.io/archive/qt/5.14/5.14.2/single/qt-everywhere-src-5.14.2.tar.xz c6fcd53c744df89e7d3223c02838a33309bd1c291fcb6f9341505fe99f7f19fa
download_and_verify http://code.breakfastquay.com/attachments/download/34/rubberband-1.8.1.tar.bz2 ff0c63b0b5ce41f937a8a3bc560f27918c5fe0b90c6bc1cb70829b86ada82b75
download_and_verify http://downloads.xiph.org/releases/libshout/libshout-2.4.1.tar.gz f3acb8dec26f2dbf6df778888e0e429a4ce9378a9d461b02a7ccbf2991bbf24d
download_and_verify https://downloads.sourceforge.net/project/libshoutidjc.idjc.p/libshout-idjc-2.4.1.tar.gz 4751c75fc85fc5d10e5b03753b046bcdee39576278bf30565c751816a87facdf
download_and_verify http://www.mega-nerd.com/libsndfile/files/libsndfile-1.0.26.tar.gz cd6520ec763d1a45573885ecb1f8e4e42505ac12180268482a44b28484a25092
download_and_verify http://prdownloads.sourceforge.net/scons/scons-2.5.0.tar.gz eb296b47f23c20aec7d87d35cfa386d3508e01d1caa3040ea6f5bbab2292ace9
download_and_verify https://www.sqlite.org/2016/sqlite-autoconf-3130000.tar.gz e2797026b3310c9d08bd472f6d430058c6dd139ff9d4e30289884ccd9744086b
download_and_verify https://taglib.github.io/releases/taglib-1.11.1.tar.gz b6d1a5a610aae6ff39d93de5efd0fdc787aa9e9dc1e7026fa4c961b26563526b
download_and_verify http://downloads.xiph.org/releases/vorbis/libvorbis-1.3.5.tar.xz 54f94a9527ff0a88477be0a71c0bab09a4c3febe0ed878b24824906cd4b0e1d1
download_and_verify https://downloads.sourceforge.net/project/lame/lame/3.100/lame-3.100.tar.gz ddfe36cab873794038ae2c1210557ad34857a4b6bdc515785d1da9e175b1da1e
download_and_verify https://download.drobilla.net/serd-0.28.0.tar.bz2 1df21a8874d256a9f3d51a18b8c6e2539e8092b62cc2674b110307e93f898aec
download_and_verify https://download.drobilla.net/sord-0.16.0.tar.bz2 9d3cb2c9966e93f537f37377171f162023cea6784ca069699be4a7770c8a035a
download_and_verify https://download.drobilla.net/sratom-0.6.0.tar.bz2 440ac2b1f4f0b7878f8b95698faa1e8f8c50929a498f68ec5d066863626a3d43
download_and_verify http://download.drobilla.net/lilv-0.24.2.tar.bz2 f7ec65b1c1f1734ded3a6c051bbaf50f996a0b8b77e814a33a34e42bce50a522
download_and_verify https://github.com/frankosterfeld/qtkeychain/archive/v0.9.1.tar.gz 9c2762d9d0759a65cdb80106d547db83c6e9fdea66f1973c6e9014f867c6f28e qtkeychain-0.9.1.tar.gz
download_and_verify http://lv2plug.in/spec/lv2-1.18.0.tar.bz2 90a3e5cf8bdca81b49def917e89fd6bba1d5845261642cd54e7888df0320473f
