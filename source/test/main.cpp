/*
File:   main.cpp
Author: Taylor Robbins
Date:   09\23\2023
Description: 
	** Holds some functions that help us test the standard library implementations without needing a project properly set up
*/

#define WASM_NEW_COMPILATION
#include "gylib/gy_defines_check.h"

#define GYLIB_LOOKUP_PRIMES_10
#include "gylib/gy.h"

#include "test_cases.cpp"

// +==============================+
// |          Initialize          |
// +==============================+
WASM_EXPORTED_FUNC(void, Initialize)
{
	// RunMathTestCases();
	// RunStringTestCases();
	RunStdLibTestCases();
	// RunStdioTestCases();
}
