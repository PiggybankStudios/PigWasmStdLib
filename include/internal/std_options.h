/*
File:   std_options.h
Author: Taylor Robbins
Date:   10\07\2023
*/

#ifndef _STD_OPTIONS_H
#define _STD_OPTIONS_H

// +--------------------------------------------------------------+
// |       STD_ASSERTIONS_ENABLED / STD_ASSERTIONS_DISABLED       |
// +--------------------------------------------------------------+
#if defined(STD_ASSERTIONS_ENABLED) && defined(STD_ASSERTIONS_DISABLED)
#error You must either define STD_ASSERTIONS_ENABLED or STD_ASSERTIONS_DISABLED, not both!
#elif defined(STD_ASSERTIONS_ENABLED)
#undef STD_ASSERTIONS_ENABLED
#define STD_ASSERTIONS_ENABLED 1
#define STD_ASSERTIONS_DISABLED 0
#elif defined(STD_ASSERTIONS_DISABLED)
#undef STD_ASSERTIONS_DISABLED
#define STD_ASSERTIONS_ENABLED 0
#define STD_ASSERTIONS_DISABLED 1
#else
#error You must define STD_ASSERTIONS_ENABLED or STD_ASSERTIONS_DISABLED before compiling PigWasm StdLib!
#endif

#endif //  _STD_OPTIONS_H

// +--------------------------------------------------------------+
// |                   Autocomplete Dictionary                    |
// +--------------------------------------------------------------+
/*
@Defines
STD_ASSERTIONS_ENABLED
STD_ASSERTIONS_DISABLED
*/
