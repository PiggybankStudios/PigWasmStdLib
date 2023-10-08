/*
File:   std_macros.h
Author: Taylor Robbins
Date:   09\23\2023
*/

#ifndef _STD_MACROS_H
#define _STD_MACROS_H

#if LANGUAGE_CPP
#define EXTERN_C extern "C"
#define EXTERN_C_START extern "C" {
#define EXTERN_C_END   }
#else
#define EXTERN_C       //nothing
#define EXTERN_C_START //nothing
#define EXTERN_C_END   //nothing
#endif

#define WASM_IMPORTED_FUNC EXTERN_C
#define WASM_EXPORTED_FUNC(returnType, functionName, ...) EXTERN_C returnType __attribute__((export_name(#functionName))) functionName(__VA_ARGS__)

#if LANGUAGE_C
#define nullptr ((void*)0)
#endif

#endif //  _STD_MACROS_H

// +--------------------------------------------------------------+
// |                   Autocomplete Dictionary                    |
// +--------------------------------------------------------------+
/*
@Defines
EXTERN_C
EXTERN_C_START
EXTERN_C_END
WASM_IMPORTED_FUNC
@Functions
#define WASM_EXPORTED_FUNC(returnType, functionName, ...)
*/
