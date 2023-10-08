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

#include "test_cases.cpp"

// +==============================+
// |          Initialize          |
// +==============================+
WASM_EXPORTED_FUNC(void, Initialize)
{
	RunMathTestCases();
}

#ifdef TEST_INCLUDE_STD_LIBRARY
#include "std_main.c"
#endif
