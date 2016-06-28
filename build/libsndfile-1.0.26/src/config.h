/* src/config.h.in.  Generated from configure.ac by autoheader.  */

// hacks
typedef int __w64 ssize_t;

/* Set to 1 if the compile is GNU GCC. */
#undef COMPILER_IS_GCC

/* Target processor clips on negative float to int conversion. */
#define CPU_CLIPS_NEGATIVE 1

/* Target processor clips on positive float to int conversion. */
#define CPU_CLIPS_POSITIVE 0

/* Target processor is big endian. */
#define CPU_IS_BIG_ENDIAN 0

/* Target processor is little endian. */
#define CPU_IS_LITTLE_ENDIAN 1

/* Set to 1 to enable experimental code. */
#define ENABLE_EXPERIMENTAL_CODE 0

/* Define to 1 if you have the <alsa/asoundlib.h> header file. */
#undef HAVE_ALSA_ASOUNDLIB_H

/* Define to 1 if you have the <byteswap.h> header file. */
#undef HAVE_BYTESWAP_H

/* Define to 1 if you have the `calloc' function. */
#define HAVE_CALLOC 1

/* Define to 1 if you have the `ceil' function. */
#define HAVE_CEIL 1

/* Set to 1 if S_IRGRP is defined. */
#define HAVE_DECL_S_IRGRP 0

/* Define to 1 if you have the <dlfcn.h> header file. */
#undef HAVE_DLFCN_H

/* Define to 1 if you have the <endian.h> header file. */
#undef HAVE_ENDIAN_H

/* Will be set to 1 if flac, ogg and vorbis are available. */
#define HAVE_EXTERNAL_LIBS 1

/* Define to 1 if you have the `floor' function. */
#define HAVE_FLOOR 1

/* Define to 1 if you have the `fmod' function. */
#define HAVE_FMOD 1

/* Define to 1 if you have the `free' function. */
#define HAVE_FREE 1

/* Define to 1 if you have the `fstat' function. */
#define HAVE_FSTAT 1

/* Define to 1 if you have the `fstat64' function. */
#define HAVE_FSTAT64 1

/* Define to 1 if you have the `fsync' function. */
#undef HAVE_FSYNC

/* Define to 1 if you have the `ftruncate' function. */
#undef HAVE_FTRUNCATE

/* Define to 1 if you have the `getpagesize' function. */
#define HAVE_GETPAGESIZE 1

/* Define to 1 if you have the `gettimeofday' function. */
#undef HAVE_GETTIMEOFDAY

/* Define to 1 if you have the `gmtime' function. */
#define HAVE_GMTIME 1

/* Define to 1 if you have the `gmtime_r' function. */
#undef HAVE_GMTIME_R

/* Define to 1 if you have the <inttypes.h> header file. */
#undef HAVE_INTTYPES_H

/* Define to 1 if you have the `m' library (-lm). */
#define HAVE_LIBM 1

/* Define to 1 if you have the <locale.h> header file. */
#undef HAVE_LOCALE_H

/* Define to 1 if you have the `localtime' function. */
#undef HAVE_LOCALTIME

/* Define to 1 if you have the `localtime_r' function. */
#undef HAVE_LOCALTIME_R

/* Define if you have C99's lrint function. */
#undef HAVE_LRINT

/* Define if you have C99's lrintf function. */
#undef HAVE_LRINTF

/* Define to 1 if you have the `lround' function. */
#undef HAVE_LROUND

/* Define to 1 if you have the `lseek' function. */
#define HAVE_LSEEK 1

/* Define to 1 if you have the `lseek64' function. */
#define HAVE_LSEEK64 1

/* Define to 1 if you have the `malloc' function. */
#define HAVE_MALLOC 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `mmap' function. */
#undef HAVE_MMAP

/* Define to 1 if you have the `open' function. */
#define HAVE_OPEN 1

/* Define to 1 if you have the `pipe' function. */
#undef HAVE_PIPE

/* Define to 1 if you have the `read' function. */
#define HAVE_READ 1

/* Define to 1 if you have the `realloc' function. */
#define HAVE_REALLOC 1

/* Define to 1 if you have the `setlocale' function. */
#undef HAVE_SETLOCALE

/* Set to 1 if <sndio.h> is available. */
#undef HAVE_SNDIO_H

/* Define to 1 if you have the `snprintf' function. */
#define HAVE_SNPRINTF 1

/* Set to 1 if you have libsqlite3. */
#define HAVE_SQLITE3 1

/* Define to 1 if the system has the type `ssize_t'. */
#undef HAVE_SSIZE_T

/* Define to 1 if you have the <stdint.h> header file. */
#undef HAVE_STDINT_H

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#undef HAVE_SYS_STAT_H

/* Define to 1 if you have the <sys/time.h> header file. */
#undef HAVE_SYS_TIME_H

/* Define to 1 if you have the <sys/types.h> header file. */
#undef HAVE_SYS_TYPES_H

/* Define to 1 if you have <sys/wait.h> that is POSIX.1 compatible. */
#undef HAVE_SYS_WAIT_H

/* Define to 1 if you have the <unistd.h> header file. */
#undef HAVE_UNISTD_H

/* Define to 1 if you have the `vsnprintf' function. */
#define HAVE_VSNPRINTF 1

/* Define to 1 if you have the `waitpid' function. */
#undef HAVE_WAITPID

/* Define to 1 if you have the `write' function. */
#define HAVE_WRITE 1

/* The host triplet of the compiled binary. */
#undef HOST_TRIPLET

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#undef LT_OBJDIR

/* The darwin version, no-zero is valid */
#undef OSX_DARWIN_VERSION

/* Set to 1 if compiling for OpenBSD */
#undef OS_IS_OPENBSD

/* Set to 1 if compiling for Win32 */
#define OS_IS_WIN32 1

/* Name of package */
#define PACKAGE "libsndfile"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "erikd@mega-nerd.com"

/* Define to the full name of this package. */
#define PACKAGE_NAME "libsndfile"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "libsndfile 1.0.26"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "libsndfile"

/* Define to the home page for this package. */
#undef PACKAGE_URL

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.0.26"

/* Set to maximum allowed value of sf_count_t type. */
//#undef SF_COUNT_MAX

/* The size of `double', as computed by sizeof. */
#define SIZEOF_DOUBLE 8

/* The size of `float', as computed by sizeof. */
#define SIZEOF_FLOAT 4

/* The size of `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of `int64_t', as computed by sizeof. */
#define SIZEOF_INT64_T 8

/* The size of `loff_t', as computed by sizeof. */
#define SIZEOF_LOFF_T sizeof(loff_t)

/* The size of `long', as computed by sizeof. */
#define SIZEOF_LONG 4

/* The size of `long long', as computed by sizeof. */
#define SIZEOF_LONG_LONG 8

/* The size of `off64_t', as computed by sizeof. */
#define SIZEOF_OFF64_T sizeof(off64_t)

/* The size of `off_t', as computed by sizeof. */
#define SIZEOF_OFF_T sizeof(off_t)

/* Set to sizeof (long) if unknown. */
#define SIZEOF_SF_COUNT_T 8

/* The size of `short', as computed by sizeof. */
#define SIZEOF_SHORT sizeof(short)

/* The size of `size_t', as computed by sizeof. */
#define SIZEOF_SIZE_T sizeof(size_t)

/* The size of `ssize_t', as computed by sizeof. */
#define SIZEOF_SSIZE_T sizeof(ssize_t)

/* The size of `void*', as computed by sizeof. */
#define SIZEOF_VOIDP sizeof(void*)

/* The size of `wchar_t', as computed by sizeof. */
#define SIZEOF_WCHAR_T 2

/* Define to 1 if you have the ANSI C header files. */
#undef STDC_HEADERS

/* Set to long if unknown. */
#define TYPEOF_SF_COUNT_T __int64

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# undef _ALL_SOURCE
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# undef _GNU_SOURCE
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# undef _POSIX_PTHREAD_SEMANTICS
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# undef _TANDEM_SOURCE
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# undef __EXTENSIONS__
#endif


/* Set to 1 to use the native windows API */
#define USE_WINDOWS_API 1

/* Version number of package */
#define VERSION "1.0.25"

/* Set to 1 if windows DLL is being built. */
#define WIN32_TARGET_DLL 0

/* Target processor is big endian. */
#undef WORDS_BIGENDIAN

/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
#undef _FILE_OFFSET_BITS

/* Define for large files, on AIX-style hosts. */
#undef _LARGE_FILES

/* Define to 1 if on MINIX. */
#undef _MINIX

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
#undef _POSIX_1_SOURCE

/* Define to 1 if you need to in order for `stat' and other things to work. */
#undef _POSIX_SOURCE

/* Set to 1 to use C99 printf/snprintf in MinGW. */
#undef __USE_MINGW_ANSI_STDIO
