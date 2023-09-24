/*
File:   std_language_defines.h
Author: Taylor Robbins
Date:   09\23\2023
*/

#ifndef _STD_LANGUAGE_DEFINES_H
#define _STD_LANGUAGE_DEFINES_H

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

#endif //  _STD_LANGUAGE_DEFINES_H

// +--------------------------------------------------------------+
// |                   Autocomplete Dictionary                    |
// +--------------------------------------------------------------+
/*
@Defines
LANGUAGE_C
LANGUAGE_CPP
*/
