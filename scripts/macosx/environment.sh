#!/bin/bash

export TARGET_I386="i386-apple-darwin10"
export TARGET_X86_64="x86_64-apple-darwin10"
export TARGET_POWERPC="powerpc-apple-darwin10"

if [[ "$1" == "i386" ]]; then
  export TARGET=$TARGET_I386
  export ARCH_FLAGS="-arch $1"
elif [[ "$1" == "x86_64" ]]; then
  export TARGET=$TARGET_X86_64
  export ARCH_FLAGS="-arch $1"
elif [[ "$1" == "ppc" ]]; then
  export TARGET=$TARGET_POWERPC
  export ARCH_FLAGS="-arch $1"
else
  echo "No architecture provided."
  # Custom $ARCH_FLAGS is set by caller.
fi

export XCODE_ROOT=$(xcode-select -print-path)
export CC="${XCODE_ROOT}/usr/bin/gcc"
export CXX="${XCODE_ROOT}/usr/bin/g++"
export CPP="$CC -E"
export CXXCPP="$CXX -E"

export OSX_SDK=${XCODE_ROOT}/Platforms/MacOSX.platform/Developer/SDKs/MacOSX${MIXXX_MACOSX_SDK}.sdk

if [[ ! -d $OSX_SDK ]]; then
    echo "WARNING: SDK path does not exist: $OSX_SDK"
    exit 1;
fi

if [[ ! -d $MIXXX_PREFIX ]]; then
    echo "WARNING: Install prefix does not exist: $MIXXX_PREFIX"
    exit 1;
fi

export PATH=$PATH:$MIXXX_PREFIX/bin/

if [[ "$DISABLE_FFAST_MATH" == "yes" ]]; then
    export COMMON_OPT_FLAGS="-O2"
else
    export COMMON_OPT_FLAGS="-O2 -ffast-math"
fi

# Core Solo and Core Duo are the only 32-bit mac machines. These support MMX, SSE, SSE2, SSE3. -march=prescott is safe to assume
export I386_OPT_FLAGS="-mmmx -msse -msse2 -msse3 -mfpmath=sse -march=prescott -mtune=generic"
export X86_64_OPT_FLAGS="-mmmx -msse -msse2 -msse3 -mfpmath=sse -mtune=generic"
export POWERPC_OPT_FLAGS=""

export OSX_CFLAGS="-isysroot $OSX_SDK -mmacosx-version-min=$MIXXX_MACOSX_TARGET $ARCH_FLAGS $COMMON_OPT_FLAGS -I$MIXXX_PREFIX/include"
export OSX_LDFLAGS="-isysroot $OSX_SDK -Wl,-syslibroot,$OSX_SDK -mmacosx-version-min=$MIXXX_MACOSX_TARGET $ARCH_FLAGS $COMMON_OPT_FLAGS -L$MIXXX_PREFIX/lib"

if [[ "$1" == "i386" ]]; then
  echo "Setting options for $1";
  export CFLAGS="$OSX_CFLAGS $I386_OPT_FLAGS"
  export CXXFLAGS="$OSX_CFLAGS $I386_OPT_FLAGS"
  export LDFLAGS="$OSX_LDFLAGS $I386_OPT_FLAGS"
  export SHLIBFLAGS="$OSX_LDFLAGS $I386_OPT_FLAGS"
  export DYLIBFLAGS="$OSX_LDFLAGS $I386_OPT_FLAGS"
elif [[ "$1" == "x86_64" ]]; then
  echo "Setting options for $1";
  export CFLAGS="$OSX_CFLAGS $X86_64_OPT_FLAGS"
  export CXXFLAGS="$OSX_CFLAGS $X86_64_OPT_FLAGS"
  export LDFLAGS="$OSX_LDFLAGS $X86_64_OPT_FLAGS"
  export SHLIBFLAGS="$OSX_LDFLAGS $X86_64_OPT_FLAGS"
  export DYLIBFLAGS="$OSX_LDFLAGS $X86_64_OPT_FLAGS"
elif [[ "$1" == "ppc" ]]; then
  echo "Setting options for $1";
  export CFLAGS="$OSX_CFLAGS $POWERPC_OPT_FLAGS"
  export CXXFLAGS="$OSX_CFLAGS $POWERPC_OPT_FLAGS"
  export LDFLAGS="$OSX_LDFLAGS $POWERPC_OPT_FLAGS"
  export SHLIBFLAGS="$OSX_LDFLAGS $POWERPC_OPT_FLAGS"
  export DYLIBFLAGS="$OSX_LDFLAGS $POWERPC_OPT_FLAGS"
else
  echo "ERROR: Unknown arch type, setting default."
  export CFLAGS=$OSX_CFLAGS
  export CXXFLAGS=$OSX_CFLAGS
  export LDFLAGS=$OSX_LDFLAGS
  export SHLIBFLAGS=$OSX_LDFLAGS
  export DYLIBFLAGS=$OSX_LDFLAGS
fi
