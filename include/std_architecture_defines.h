/*
File:   std_architecture_defines.h
Author: Taylor Robbins
Date:   09\23\2023
*/

#ifndef _STD_ARCHITECTURE_DEFINES_H
#define _STD_ARCHITECTURE_DEFINES_H

// +--------------------------------------------------------------+
// |                   ARCH_NATIVE / ARCH_WASM                    |
// +--------------------------------------------------------------+
#if !defined(ARCH_NATIVE) && !defined(ARCH_WASM)
#error You must define ARCH_NATIVE or ARCH_WASM
#endif

#ifdef ARCH_NATIVE
#undef ARCH_NATIVE
#define ARCH_NATIVE 1
#else
#define ARCH_NATIVE 0
#endif

#ifdef ARCH_WASM
#undef ARCH_WASM
#define ARCH_WASM 1
#else
#define ARCH_WASM 0
#endif

#endif //  _STD_ARCHITECTURE_DEFINES_H

// +--------------------------------------------------------------+
// |                   Autocomplete Dictionary                    |
// +--------------------------------------------------------------+
/*
@Defines
ARCH_NATIVE
ARCH_WASM
*/
