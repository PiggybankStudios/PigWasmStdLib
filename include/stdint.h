/*
File:   stdint.h
Author: Taylor Robbins
Date:   09\23\2023
NOTE: This file cannot depend on anything but std_common.h
*/

#ifndef _STDINT_H
#define _STDINT_H

#include "std_common.h"

#if ARCH_WASM
	#define _Addr long
	#define _Int64 long long
	#define _Reg long
	#define __BYTE_ORDER __LITTLE_ENDIAN
	#define __LONG_MAX 0x7FFFFFFFFFFFFFFFL
	#ifndef __cplusplus
	#ifdef __WCHAR_TYPE__
	typedef __WCHAR_TYPE__ wchar_t;
	#else
	typedef long wchar_t;
	#endif
	#endif
#elif ARCH_NATIVE
	#define _Addr int
	#define _Int64 long long
	#define _Reg long long
	#define __BYTE_ORDER __LITTLE_ENDIAN
	#define __LONG_MAX 0x7FFFFFFFL
	#ifndef __cplusplus
	typedef int wchar_t;
	#endif
#else
#error Unsupported architecture in stdint.h
#endif

typedef float  float_t;
typedef double double_t;

#if !COMPILER_MSVC
typedef unsigned _Addr size_t;
#endif
typedef unsigned _Addr uintptr_t;
typedef _Addr          ptrdiff_t;
typedef _Addr          ssize_t;
typedef _Addr          intptr_t;
// typedef _Addr          regoff_t;
// typedef _Reg           register_t;
// typedef _Int64         time_t;
// typedef _Int64         suseconds_t;

typedef signed char     int8_t;
typedef signed short    int16_t;
typedef signed int      int32_t;
typedef signed _Int64   int64_t;
typedef signed _Int64   intmax_t;
typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;
typedef unsigned _Int64 uint64_t;
typedef unsigned _Int64 u_int64_t;
typedef unsigned _Int64 uintmax_t;

// typedef unsigned        mode_t;
// typedef unsigned _Reg   nlink_t;
// typedef _Int64          off_t;
// typedef unsigned _Int64 ino_t;
// typedef unsigned _Int64 dev_t;
// typedef long            blksize_t;
// typedef _Int64          blkcnt_t;
// typedef unsigned _Int64 fsblkcnt_t;
// typedef unsigned _Int64 fsfilcnt_t;

// typedef unsigned      wint_t;
// typedef unsigned long wctype_t;

// typedef void *  timer_t;
// typedef int     clockid_t;
// typedef long    clock_t;
// struct timeval  { time_t tv_sec; suseconds_t tv_usec; };
// struct timespec { time_t tv_sec; int :8*(sizeof(time_t)-sizeof(long))*(__BYTE_ORDER==__BIG_ENDIAN); long tv_nsec; int :8*(sizeof(time_t)-sizeof(long))*(__BYTE_ORDER!=__BIG_ENDIAN); };

// typedef int      pid_t;
// typedef unsigned id_t;
// typedef unsigned uid_t;
// typedef unsigned gid_t;
// typedef int      key_t;
// typedef unsigned useconds_t;

#endif //  _STDINT_H