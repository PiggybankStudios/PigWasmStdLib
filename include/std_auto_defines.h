/*
File:   std_auto_defines.h
Author: Taylor Robbins
Date:   09\23\2023
Description:
	** These defines are configured automatically using compiler driven defines
*/

#ifndef _STD_AUTO_DEFINES_H
#define _STD_AUTO_DEFINES_H

// +--------------------------------------------------------------+
// |                  LANGUAGE_C / LANGUAGE_CPP                   |
// +--------------------------------------------------------------+
#ifdef __cplusplus
#define LANGUAGE_C   0
#define LANGUAGE_CPP 1
#else
#define LANGUAGE_C   1
#define LANGUAGE_CPP 0
#endif

#ifndef __wasm__
#error This standard library implementation only works for WebAssembly!
#endif

#ifndef __clang__
#error This standard library implementation only works with the clang compiler!
#endif

#if !defined(__clang_major__) || !defined(__clang_minor__) || !defined(__clang_patchlevel__)
#error Missing defines for __clang_major__, __clang_minor__, and __clang_patchlevel__!
#endif
 
#if __clang_major__ > 13 || (__clang_major__ == 13 && __clang_minor__ > 0) || (__clang_major__ == 13 && __clang_minor__ == 0 && __clang_patchlevel__ > 1)
#warning WARNING: This standard library implementation has not been tested with versions of clang past 13.0.1
#endif

#if __clang_major__ < 13 || __clang_patchlevel__ < 1
#error This standard library implementation does not support versions of clang before 13.0.1!
#endif

#ifndef __BYTE_ORDER__
#error Missing __BYTE_ORDER__ define!
#elif __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error This standard library implementation assumes little-endian byte order
#endif

#endif //  _STD_AUTO_DEFINES_H

// +--------------------------------------------------------------+
// |                   Autocomplete Dictionary                    |
// +--------------------------------------------------------------+
/*
@Defines
LANGUAGE_C
LANGUAGE_CPP
*/
