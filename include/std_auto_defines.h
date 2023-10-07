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

#endif //  _STD_AUTO_DEFINES_H

// +--------------------------------------------------------------+
// |                   Autocomplete Dictionary                    |
// +--------------------------------------------------------------+
/*
@Defines
LANGUAGE_C
LANGUAGE_CPP
*/
