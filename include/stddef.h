/*
File:   stddef.h
Author: Taylor Robbins
Date:   10\12\2023
*/

#ifndef _STDDEF_H
#define _STDDEF_H

#include <internal/std_common.h>

#include <stdint.h>

#if LANGUAGE_CPP
#define NULL nullptr
#else
#define NULL ((void*)0)
#endif

#define offsetof(type, member) __builtin_offsetof(type, member)

#endif //  _STDDEF_H

// +--------------------------------------------------------------+
// |                   Autocomplete Dictionary                    |
// +--------------------------------------------------------------+
/*
@Defines
NULL
@Functions
#define offsetof(type, member)
*/
