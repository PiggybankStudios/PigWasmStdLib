/*
File:   main.c
Author: Taylor Robbins
Date:   09\23\2023
Description: 
	** Holds some functions that help us test the standard library implementations without needing a project properly set up
*/

#include <math.h>
// #include <float.h>

// #include "gy_defines_check.h"

// #define GYLIB_LOOKUP_PRIMES_10
// #include "gy.h"

int main()
{
	// int bigArray[4096];
	// bigArray[0] = 10;
	
	return 0;
}

#if OS_WINDOWS
int __stdcall WinMainCRTStartup()
{
	int result = main();
	return result;
}
#else
int __stdcall mainCRTStartup()
{
	int result = main();
	return result;
}
#endif
