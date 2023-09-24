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

#include "std_os_defines.h"
#include "std_compiler_defines.h"
#include "std_architecture_defines.h"
#include "std_language_defines.h"

#include "std_macros.h"

#include "stdint.h"
#include "float.h"

#endif //  _STD_COMMON_H
