/*
File:   stdio.h
Author: Taylor Robbins
Date:   10\12\2023
*/

#ifndef _STDIO_H
#define _STDIO_H

#include "std_common.h"

EXTERN_C_START

int vsnprintf(char* bufferPntr, size_t bufferSize, const char* formatStr, va_list args);

EXTERN_C_END

#endif //  _STDIO_H
