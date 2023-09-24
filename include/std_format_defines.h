/*
File:   std_format_defines.h
Author: Taylor Robbins
Date:   09\23\2023
*/

#ifndef _STD_FORMAT_DEFINES_H
#define _STD_FORMAT_DEFINES_H

// +--------------------------------------------------------------+
// |               STD_FORMAT_LIB / STD_FORMAT_DLL                |
// +--------------------------------------------------------------+
#if !defined(STD_FORMAT_LIB) && !defined(STD_FORMAT_DLL)
#error You must define STD_FORMAT_LIB or STD_FORMAT_DLL
#endif

#ifdef STD_FORMAT_LIB
#undef STD_FORMAT_LIB
#define STD_FORMAT_LIB 1
#else
#define STD_FORMAT_LIB 0
#endif

#ifdef STD_FORMAT_DLL
#undef STD_FORMAT_DLL
#define STD_FORMAT_DLL 1
#else
#define STD_FORMAT_DLL 0
#endif

#endif //  _STD_FORMAT_DEFINES_H

// +--------------------------------------------------------------+
// |                   Autocomplete Dictionary                    |
// +--------------------------------------------------------------+
/*
@Defines
STD_FORMAT_LIB
STD_FORMAT_DLL
*/
