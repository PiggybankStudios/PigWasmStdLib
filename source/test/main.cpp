/*
File:   main.cpp
Author: Taylor Robbins
Date:   09\23\2023
Description: 
	** Holds some functions that help us test the standard library implementations without needing a project properly set up
*/

#if 1
#include <limits.h>
#include <stdint.h>
#include <float.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <intrin.h>
#include <stdio.h>
#else
#include "gylib/gy_defines_check.h"
#define GYLIB_LOOKUP_PRIMES_10
#include "gylib/gy.h"
#endif

WASM_IMPORTED_FUNC void jsPrintInteger(const char* labelStrPntr, int number);
WASM_IMPORTED_FUNC void jsPrintFloat(const char* labelStrPntr, double number);
WASM_IMPORTED_FUNC void jsPrintString(const char* labelStrPntr, const char* strPntr);

#include "test_cases.cpp"

// +==============================+
// |          Initialize          |
// +==============================+
WASM_EXPORTED_FUNC(void, Initialize)
{
	// RunMathTestCases();
	RunStringTestCases();
	// RunStdLibTestCases();
	// RunStdioTestCases();
}
