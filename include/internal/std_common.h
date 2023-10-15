/*
File:   std_common.h
Author: Taylor Robbins
Date:   09\23\2023
Description:
	** Included FIRST in all other header files.
	** Defines the list of includes that are pulled in no matter what header file the user includes first
*/

#ifndef _STD_COMMON_H
#define _STD_COMMON_H

#define USING_PIG_WASM_STD_LIB   1

#include "internal/std_auto_defines.h"
#include "internal/std_options.h"
#include "internal/std_constants.h"
#include "internal/std_macros.h"
#include "internal/std_js_api.h"

#endif //  _STD_COMMON_H
