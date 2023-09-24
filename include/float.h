/*
File:   float.h
Author: Taylor Robbins
Date:   09\23\2023
NOTE: This file cannot depend on anything but std_common.h
*/

#ifndef _FLOAT_H
#define _FLOAT_H

#include "std_common.h"

#if ARCH_WASM
	//TODO: Is this really true for WASM?
	#define LDBL_TRUE_MIN 6.47517511943802511092443895822764655e-4966L
	#define LDBL_MIN      3.36210314311209350626267781732175260e-4932L
	#define LDBL_MAX      1.18973149535723176508575932662800702e+4932L
	#define LDBL_EPSILON  1.92592994438723585305597794258492732e-34L
	
	#define LDBL_MANT_DIG 113
	#define LDBL_MIN_EXP  (-16381)
	#define LDBL_MAX_EXP  16384
	
	#define LDBL_DIG        33
	#define LDBL_MIN_10_EXP (-4931)
	#define LDBL_MAX_10_EXP 4932
	
	#define DECIMAL_DIG 36
#elif ARCH_NATIVE
	#define LDBL_TRUE_MIN 6.47517511943802511092443895822764655e-4966L
	#define LDBL_MIN      3.36210314311209350626267781732175260e-4932L
	#define LDBL_MAX      1.18973149535723176508575932662800702e+4932L
	#define LDBL_EPSILON  1.92592994438723585305597794258492732e-34L
	
	#define LDBL_MANT_DIG 113
	#define LDBL_MIN_EXP  (-16381)
	#define LDBL_MAX_EXP  16384
	
	#define LDBL_DIG        33
	#define LDBL_MIN_10_EXP (-4931)
	#define LDBL_MAX_10_EXP 4932
	
	#define DECIMAL_DIG 36
#else
#error Unsupported architecture in float.h
#endif

#endif //  _FLOAT_H

// +--------------------------------------------------------------+
// |                   Autocomplete Dictionary                    |
// +--------------------------------------------------------------+
/*
@Defines
LDBL_TRUE_MIN
LDBL_MIN
LDBL_MAX
LDBL_EPSILON
LDBL_MANT_DIG
LDBL_MIN_EXP
LDBL_MAX_EXP
LDBL_DIG
LDBL_MIN_10_EXP
LDBL_MAX_10_EXP
DECIMAL_DIG
*/
