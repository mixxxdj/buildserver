/*
** Timing functions.
** 
** This program is distributed under the GNU General Public License, version 2.
** A copy of this license is included with this source.
*/

#ifndef __TIMING_H__
#define __TIMING_H__

#if defined(_WIN32)
#define HAVE_FTIME 1
#include <sys/timeb.h>
#endif

#include <sys/types.h>
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#if defined(_WIN32) && !defined(int64_t)
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif

/* config.h should be included before we are to define _mangle */
#ifdef _mangle
# define timing_get_time _mangle(timing_get_time)
# define timing_sleep _mangle(timing_sleep)
#endif

uint64_t timing_get_time(void);
void timing_sleep(uint64_t sleeptime);

#endif  /* __TIMING_H__ */
