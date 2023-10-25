/*
File:   stdio.h
Author: Taylor Robbins
Date:   10\12\2023
*/

#ifndef _STDIO_H
#define _STDIO_H

#include <internal/std_common.h>

#include <stdint.h>
#include <stdarg.h>

CONDITIONAL_EXTERN_C_START

int vsnprintf(char* bufferPntr, size_t bufferSize, const char* formatStr, va_list args);

CONDITIONAL_EXTERN_C_END

#endif //  _STDIO_H
