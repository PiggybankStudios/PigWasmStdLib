/*
File:   stdio.c
Author: Taylor Robbins
Date:   10\12\2023
Description: 
	** Holds all the implementations for functions declared in stdio.h
*/

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

int vsnprintf(char* bufferPntr, size_t bufferSize, const char* formatStr, va_list args)
{
	assert(bufferSize <= INT_MAX);
	return stbsp_vsnprintf(bufferPntr, bufferSize, formatStr, args);
}