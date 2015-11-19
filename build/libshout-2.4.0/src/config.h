#if defined(_WIN32) || defined(_WIN64) 
#define HAVE_STDINT_H 1
#define HAVE_SYS_TIMEB_H 1
#define HAVE_FTIME 1

#include <os.h>
typedef uint32_t socklen_t;

#include <stdio.h>
#include <stdarg.h>
#include <process.h>
#define snprintf _snprintf 
#define vsnprintf _vsnprintf 
#define strcasecmp _stricmp 
#define strncasecmp _strnicmp 
#define getpid _getpid
#define inline __inline
#endif