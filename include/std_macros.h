/*
File:   std_macros.h
Author: Taylor Robbins
Date:   09\23\2023
*/

#ifndef _STD_MACROS_H
#define _STD_MACROS_H

#if LANGUAGE_CPP
#define EXTERN_C_START extern "C" {
#define EXTERN_C_END   }
#else
#define EXTERN_C_START //nothing
#define EXTERN_C_END   //nothing
#endif

#endif //  _STD_MACROS_H

// +--------------------------------------------------------------+
// |                   Autocomplete Dictionary                    |
// +--------------------------------------------------------------+
/*
@Defines
EXTERN_C_START
EXTERN_C_END
*/
