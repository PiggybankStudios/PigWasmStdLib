/*
File:   float.h
Author: Taylor Robbins
Date:   09\23\2023
NOTE: This file cannot depend on anything but std_common.h
*/

#ifndef _FLOAT_H
#define _FLOAT_H

#include "std_common.h"

EXTERN_C_START

//TODO: Can we comment all of these defines to describe in detail what the values are?

// inline int __flt_rounds() { return FE_TONEAREST; }
// #define FLT_ROUNDS       (__flt_rounds())

#define FLT_EVAL_METHOD  __FLT_EVAL_METHOD__
#define FLT_RADIX        2

#define FLT_TRUE_MIN     1.40129846432481707092e-45F
#define FLT_MIN          1.17549435082228750797e-38F
#define FLT_MAX          3.40282346638528859812e+38F
#define FLT_EPSILON      1.1920928955078125e-07F

#define FLT_MANT_DIG     24
#define FLT_MIN_EXP      (-125)
#define FLT_MAX_EXP      128
#define FLT_HAS_SUBNORM  1

#define FLT_DIG          6
#define FLT_DECIMAL_DIG  9
#define FLT_MIN_10_EXP   (-37)
#define FLT_MAX_10_EXP   38

#define DBL_TRUE_MIN     4.94065645841246544177e-324
#define DBL_MIN          2.22507385850720138309e-308
#define DBL_MAX          1.79769313486231570815e+308
#define DBL_EPSILON      2.22044604925031308085e-16

#define DBL_MANT_DIG     53
#define DBL_MIN_EXP      (-1021)
#define DBL_MAX_EXP      1024
#define DBL_HAS_SUBNORM  1

#define DBL_DIG          15
#define DBL_DECIMAL_DIG  17
#define DBL_MIN_10_EXP   (-307)
#define DBL_MAX_10_EXP   308

//NOTE: long double is an allowed type in WASM32 but casting that long double to double results in an import for __trunctfdf2 being generated. We're just not going to use long doubles for now...
//TODO: We are turning these back on and using them for __floatscan which drives musl's implementation for atof
#if 1
#define LDBL_HAS_SUBNORM 1
#define LDBL_DECIMAL_DIG DECIMAL_DIG

#define LDBL_TRUE_MIN    6.47517511943802511092443895822764655e-4966L
#define LDBL_MIN         3.36210314311209350626267781732175260e-4932L
#define LDBL_MAX         1.18973149535723176508575932662800702e+4932L
#define LDBL_EPSILON     1.92592994438723585305597794258492732e-34L

#define LDBL_MANT_DIG    113
#define LDBL_MIN_EXP     (-16381)
#define LDBL_MAX_EXP     16384

#define LDBL_DIG         33
#define LDBL_MIN_10_EXP  (-4931)
#define LDBL_MAX_10_EXP  4932
#endif

#define DECIMAL_DIG      36

EXTERN_C_END

#endif //  _FLOAT_H

// +--------------------------------------------------------------+
// |                   Autocomplete Dictionary                    |
// +--------------------------------------------------------------+
/*
@Defines
FLT_RADIX
FLT_TRUE_MIN
FLT_MIN
FLT_MAX
FLT_EPSILON
FLT_MANT_DIG
FLT_MIN_EXP
FLT_MAX_EXP
FLT_HAS_SUBNORM
FLT_DIG
FLT_DECIMAL_DIG
FLT_MIN_10_EXP
FLT_MAX_10_EXP
DBL_TRUE_MIN
DBL_MIN
DBL_MAX
DBL_EPSILON
DBL_MANT_DIG
DBL_MIN_EXP
DBL_MAX_EXP
DBL_HAS_SUBNORM
DBL_DIG
DBL_DECIMAL_DIG
DBL_MIN_10_EXP
DBL_MAX_10_EXP
DECIMAL_DIG
*/
