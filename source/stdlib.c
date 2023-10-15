/*
File:   stdlib.c
Author: Taylor Robbins
Date:   10\07\2023
Description: 
	** Holds all the implementations for functions declared in stdlib.h
*/

#define PIG_WASM_STD_USE_CHATGPT_ATOF  0

int abs(int value)
{
	return ((value > 0) ? value : -value);
}

void* malloc(size_t numBytes)
{
	return WasmMemoryAllocate(numBytes);
}

void* calloc(size_t numElements, size_t elemSize)
{
	assert(0 && "callocing memory is not allowed in WebAssembly!");
	return nullptr;
}

void* realloc(void* prevAllocPntr, size_t newSize)
{
	assert(0 && "reallocing memory is not allowed in WebAssembly!");
	return nullptr;
}

void free(void* allocPntr)
{
	assert(0 && "freeing memory is not allowed in WebAssembly!");
}

void* aligned_alloc(size_t numBytes, size_t alignmentSize)
{
	assert(0 && "aligned_allocing memory is not allowed in WebAssembly!");
	return nullptr;
}

static uint64_t stdRandState;

void srand(unsigned int seed)
{
	stdRandState = (seed - 1);
}

int rand()
{
	// Similar to our 64-bit Permuted Congruential in gylib?
	stdRandState = (6364136223846793005ULL * stdRandState) + 1;
	return (stdRandState >> 33);
}

#if PIG_WASM_STD_USE_CHATGPT_ATOF
double chatgpt_atof(const char *str)
{
	// This function was written by ChatGPT using the following prompt:
	// Can you write an implementation for atof in C that doesn't use anything from the C standard library? The atof function takes a const char* and produces a double result, with the double being the value of the number represented by the string. Also make sure it handles any invalid characters and returns 0 if it finds any invalid characters or syntax
	double result = 0.0;
	double fraction = 0.1;
	int sign = 1;
	int state = 0; // 0 for integer part, 1 for fractional part
	int invalid = 0; // Flag for invalid characters
	
	while (*str)
	{
		if (state == 0 && *str == '-') { sign = -1; }
		else if (state == 0 && *str == '+') { } // Ignore the plus sign, if present
		else if (*str >= '0' && *str <= '9')
		{
			if (state == 0) { result = result * 10.0 + (*str - '0'); }
			else
			{
				result += fraction * (*str - '0');
				fraction *= 0.1;
			}
		}
		else if (*str == '.' && state == 0) { state = 1; }
		else { invalid = 1; }
		str++;
	}
	
	if (invalid) { return 0.0; } // Invalid character encountered
	
	return result * sign;
}
#endif

#define ATOF_MAX_STR_LENGTH 1000
#define atof_IsDigit(c) (c >= '0' && c <= '9')
#define atof_CharValue(c) (c - '0')

double atof(const char* str)
{
	#if PIG_WASM_STD_USE_CHATGPT_ATOF
	return chatgpt_atof(str);
	#else
	int sign = 1;
	double wholePart = 0;
	double fractionalPart = 0;
	int exponentPartSign = 1;
	int exponentPart = 0;
	char currentChar = *str++;
	int charIndex = 0; //Limit for security purposes
	
	while (currentChar == ' ') { currentChar = *str++; }
	
	if (currentChar == '+' || currentChar == '-')
	{
		sign = ((currentChar == '-') ? -1 : 1);
		currentChar = *str++;
	}
	
	while (currentChar != '\0' && atof_IsDigit(currentChar) && charIndex < ATOF_MAX_STR_LENGTH)
	{
		wholePart = (wholePart * 10) + atof_CharValue(currentChar);
		currentChar = *str++;
		charIndex++;
	}
	
	if (currentChar == '.' || currentChar == ',')
	{
		currentChar = *str++;
		
		double fracPower = 1;
		while (currentChar != '\0' && atof_IsDigit(currentChar) && charIndex < ATOF_MAX_STR_LENGTH)
		{
			fracPower *= 10;
			fractionalPart += atof_CharValue(currentChar) / fracPower;
			currentChar = *str++;
			charIndex++;
		}
	}
	
	if (currentChar == 'e' || currentChar == 'E')
	{
		currentChar = *str++;
		
		if (currentChar == '+' || currentChar == '-')
		{
			exponentPartSign = ((currentChar == '-') ? -1 : 1);
			currentChar = *str++;
		}
		
		while (currentChar != '\0' && atof_IsDigit(currentChar) && charIndex < ATOF_MAX_STR_LENGTH)
		{
			exponentPart = (exponentPart * 10) + atof_CharValue(currentChar);
			currentChar = *str++;
			charIndex++;
		}
	}
	
	while (currentChar == ' ') { currentChar = *str++; }
	
	if (currentChar != '\0')
	{
		//TODO: Set errno? We either hit our limit (charIndex >= ATOF_MAX_STR_LENGTH) or found an invalid character
		return 0.0;
	}
	
	double result = sign * (wholePart + fractionalPart);
	for (int eIndex = 0; eIndex < exponentPart; eIndex++) { result = result * (exponentPartSign > 0 ? 10.0 : 0.1f); }
	return result;
	#endif
}

void* alloca(size_t numBytes)
{
	return __builtin_alloca(numBytes);
}

#include "stdlib_qsort_helpers.c"

void qsort(void* basePntr, size_t numItems, size_t itemSize, StdCompareFunc_f* compareFunc)
{
	__qsort_r(basePntr, numItems, itemSize, WrapperCompareFunc, compareFunc);
}
