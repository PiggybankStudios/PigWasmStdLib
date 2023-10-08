/*
File:   main.cpp
Author: Taylor Robbins
Date:   09\23\2023
Description: 
	** Holds some functions that help us test the standard library implementations without needing a project properly set up
*/

#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <math.h>
#include <assert.h>

// #include "gylib/gy_defines_check.h"

// #define GYLIB_LOOKUP_PRIMES_10
// #include "gylib/gy.h"

// +==============================+
// |          Initialize          |
// +==============================+
WASM_EXPORTED_FUNC(void, Initialize)
{
	// float testFloat = 1.02f;
	// if (isnan(testFloat)) { jsPrintString("It's NaN!"); }
	// if (isinf(testFloat)) { jsPrintString("It's Inifite!"); }
	// if (isnormal(testFloat)) { jsPrintString("It's normal!"); }
	
	double largeFloat1 = -13.0123131414123123;
	// jsPrintNumber("sizeof(long double)", sizeof(long double));
	jsPrintFloat("fmod", fmod(largeFloat1, 1.0f));
	// if (isnan(largeFloat)) { jsPrintString("It's NaN!"); }
}

#ifdef TEST_INCLUDE_STD_LIBRARY
#include "std_main.c"
#endif
