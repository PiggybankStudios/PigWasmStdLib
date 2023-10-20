/*
File:   std_macros.h
Author: Taylor Robbins
Date:   09\23\2023
*/

#ifndef _STD_MACROS_H
#define _STD_MACROS_H

#if LANGUAGE_CPP
#define CONDITIONAL_EXTERN_C extern "C"
#define CONDITIONAL_EXTERN_C_START extern "C" {
#define CONDITIONAL_EXTERN_C_END   }
#else
#define CONDITIONAL_EXTERN_C       //nothing
#define CONDITIONAL_EXTERN_C_START //nothing
#define CONDITIONAL_EXTERN_C_END   //nothing
#endif

//TODO: __attribute__((import_module("env"), import_name(#functionName)))
#define WASM_IMPORTED_FUNC CONDITIONAL_EXTERN_C
#define WASM_EXPORTED_FUNC(returnType, functionName, ...) CONDITIONAL_EXTERN_C returnType __attribute__((export_name(#functionName))) functionName(__VA_ARGS__)

#if LANGUAGE_C
#define nullptr ((void*)0)
#endif

#endif //  _STD_MACROS_H

// +--------------------------------------------------------------+
// |                   Autocomplete Dictionary                    |
// +--------------------------------------------------------------+
/*
@Defines
CONDITIONAL_EXTERN_C
CONDITIONAL_EXTERN_C_START
CONDITIONAL_EXTERN_C_END
WASM_IMPORTED_FUNC
@Functions
#define WASM_EXPORTED_FUNC(returnType, functionName, ...)
*/
