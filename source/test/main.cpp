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
#include <stdbool.h>

#define WASM_NEW_COMPILATION
#include "gylib/gy_defines_check.h"

#define GYLIB_LOOKUP_PRIMES_10
#include "gylib/gy.h"

#include "test_cases.cpp"

int AssemblyFunc(int x)
{
	int myLocalVar = 213;
	__asm__ __volatile__
	(
		"i32.const 15\n"
		"i32.const 17\n"
		"i32.add\n"
		// "i32.store offset=8\n"
	);
	return myLocalVar;
}

// +==============================+
// |          Initialize          |
// +==============================+
WASM_EXPORTED_FUNC(void, Initialize)
{
	// jsPrintNumber("assemblyResult", AssemblyFunc(42));
	// int someNumber = 10;
	// memset(&someNumber, 0x01, sizeof(someNumber));
	// jsPrintNumber("someNumber", someNumber);
	
	// RunMathTestCases();
	// RunStringTestCases();
	// RunStdLibTestCases();
	RunPrintTestCases();
}

#ifdef TEST_INCLUDE_STD_LIBRARY
#include "std_main.c"
#endif
