/*
File:   string.h
Author: Taylor Robbins
Date:   10\12\2023
*/

#ifndef _STRING_H
#define _STRING_H

#include "std_common.h"

EXTERN_C_START

void* _memset(void* pntr, int value, size_t numBytes);
#define memset(pntr, value, numBytes) _memset((pntr), (value), (numBytes))
int memcmp(const void* left, const void* right, size_t numBytes);
void* _memcpy(void* dest, const void* source, size_t numBytes);
#define memcpy(dest, source, numBytes) _memcpy((dest), (source), (numBytes))

int strncmp(const char* left, const char* right, size_t numBytes);
size_t strlen(const char* str);
size_t wcslen(const wchar_t* str);

EXTERN_C_END

#endif //  _STRING_H

// +--------------------------------------------------------------+
// |                   Autocomplete Dictionary                    |
// +--------------------------------------------------------------+
/*
@Functions
void* memset(void* pntr, int value, size_t numBytes)
int memcmp(const void* left, const void* right, size_t numBytes)
void* memcpy(void* dest, const void* source, size_t numBytes)
int strncmp(const char* left, const char* right, size_t numBytes)
size_t strlen(const char* str)
size_t wcslen(const wchar_t* str)
*/
