#!/usr/bin/env bash

set -e 

DEPENDENCIES=dependencies
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
download_and_verify http://ftp.gnu.org/gnu/automake/automake-1.15.tar.xz 9908c75aabd49d13661d6dcb1bc382252d22cc77bf733a2d55e87f2aa2db8636
download_and_verify http://ftp.wayne.edu/gnu/libtool/libtool-2.4.tar.xz afcce660d3dc54c63a0a5ba3cf05272239dc3c54bbeba20f6bad250f9dc007ae
download_and_verify https://bitbucket.org/acoustid/chromaprint/downloads/chromaprint-1.3.1.tar.gz 8f95a011e6fe3a75281520daa9376b7f028300e20a5be9af08ef9c851d4bb581
download_and_verify https://cmake.org/files/v3.5/cmake-3.5.2.tar.gz 92d8410d3d981bb881dfff2aed466da55a58d34c7390d50449aa59b32bb5e62a
download_and_verify http://downloads.xiph.org/releases/flac/flac-1.3.1.tar.xz 4773c0099dba767d963fd92143263be338c48702172e8754b9bc5103efe1c56c
download_and_verify http://downloads.mixxx.org/hss1394-r8.tar.gz d5c87a92ad8e277b418300fbbfcc06a0826dfde71f7b10643ac59703d795c002
download_and_verify https://www.openssl.org/source/openssl-1.0.2h.tar.gz 1d4007e53aad94a5b2002fe045ee7bb0b3d98f1a47f8b2bc851dcd1c74332919
download_and_verify http://downloads.xiph.org/releases/ogg/libogg-1.3.2.tar.xz 3f687ccdd5ac8b52d76328fbbfebc70c459a40ea891dbf3dccb74a210826e79b
download_and_verify http://downloads.xiph.org/releases/opus/opus-1.1.2.tar.gz 0e290078e31211baa7b5886bcc8ab6bc048b9fc83882532da4a1a45e58e907fd
download_and_verify http://downloads.xiph.org/releases/opus/opusfile-0.7.tar.gz 9e2bed13bc729058591a0f1cab2505e8cfd8e7ac460bf10a78bcc3b125e7c301
download_and_verify http://www.portaudio.com/archives/pa_stable_v19_20140130.tgz 8fe024a5f0681e112c6979808f684c3516061cc51d3acc0b726af98fc96c8d57
download_and_verify http://downloads.sourceforge.net/project/portmedia/portmidi/217/portmidi-src-217.zip 08e9a892bd80bdb1115213fb72dc29a7bf2ff108b378180586aa65f3cfd42e0f
download_and_verify https://github.com/google/protobuf/archive/v2.6.1.tar.gz 2667b7cda4a6bc8a09e5463adf3b5984e08d94e72338277affa8594d8b6e5cd1 protobuf-2.6.1.tar.gz
download_and_verify https://download.qt.io/archive/qt/4.8/4.8.6/qt-everywhere-opensource-src-4.8.6.tar.gz 8b14dd91b52862e09b8e6a963507b74bc2580787d171feda197badfa7034032c
download_and_verify https://download.qt.io/archive/qt/5.6/5.6.0/single/qt-everywhere-opensource-src-5.6.0.tar.gz 7716bf4b231ab1768676a49e19781b5e03b27b9068d6685857e5ec43e9f836bf
download_and_verify http://code.breakfastquay.com/attachments/download/34/rubberband-1.8.1.tar.bz2 ff0c63b0b5ce41f937a8a3bc560f27918c5fe0b90c6bc1cb70829b86ada82b75
download_and_verify http://downloads.xiph.org/releases/libshout/libshout-2.4.1.tar.gz f3acb8dec26f2dbf6df778888e0e429a4ce9378a9d461b02a7ccbf2991bbf24d
download_and_verify http://www.mega-nerd.com/libsndfile/files/libsndfile-1.0.26.tar.gz cd6520ec763d1a45573885ecb1f8e4e42505ac12180268482a44b28484a25092
download_and_verify http://prdownloads.sourceforge.net/scons/scons-2.5.0.tar.gz eb296b47f23c20aec7d87d35cfa386d3508e01d1caa3040ea6f5bbab2292ace9
download_and_verify https://www.sqlite.org/2016/sqlite-autoconf-3130000.tar.gz e2797026b3310c9d08bd472f6d430058c6dd139ff9d4e30289884ccd9744086b
download_and_verify https://taglib.github.io/releases/taglib-1.11.tar.gz ed4cabb3d970ff9a30b2620071c2b054c4347f44fc63546dbe06f97980ece288
download_and_verify http://downloads.xiph.org/releases/vorbis/libvorbis-1.3.5.tar.xz 54f94a9527ff0a88477be0a71c0bab09a4c3febe0ed878b24824906cd4b0e1d1
