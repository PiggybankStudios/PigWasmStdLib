/*
File:   std_compiler_defines.h
Author: Taylor Robbins
Date:   09\23\2023
*/

#ifndef _STD_COMPILER_DEFINES_H
#define _STD_COMPILER_DEFINES_H

// +--------------------------------------------------------------+
// |        COMPILER_MSVC / COMPILER_CLANG / COMPILER_GCC         |
// +--------------------------------------------------------------+
#if !defined(COMPILER_MSVC) && !defined(COMPILER_CLANG) && !defined(COMPILER_GCC)
#error You must define COMPILER_MSVC, COMPILER_CLANG or COMPILER_GCC
#endif

#ifdef COMPILER_MSVC
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#else
#define COMPILER_MSVC 0
#endif

#ifdef COMPILER_CLANG
#undef COMPILER_CLANG
#define COMPILER_CLANG 1
#else
#define COMPILER_CLANG 0
#endif

#ifdef COMPILER_GCC
#undef COMPILER_GCC
#define COMPILER_GCC 1
#else
#define COMPILER_GCC 0
#endif

#endif //  _STD_COMPILER_DEFINES_H

// +--------------------------------------------------------------+
// |                   Autocomplete Dictionary                    |
// +--------------------------------------------------------------+
/*
@Defines
COMPILER_MSVC
COMPILER_CLANG
COMPILER_GCC
*/
