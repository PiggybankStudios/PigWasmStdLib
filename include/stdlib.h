/*
File:   stdlib.h
Author: Taylor Robbins
Date:   10\07\2023
*/

#ifndef _STDLIB_H
#define _STDLIB_H

#include "std_common.h"
#include "stdint.h"

EXTERN_C_START

void* malloc(size_t numBytes);
void* calloc(size_t numElements, size_t elemSize);
void* realloc(void* prevAllocPntr, size_t newSize);
void free(void* allocPntr);
void* aligned_alloc(size_t numBytes, size_t alignmentSize);

EXTERN_C_END

#endif //  _STDLIB_H

// +--------------------------------------------------------------+
// |                   Autocomplete Dictionary                    |
// +--------------------------------------------------------------+
/*
@Functions
void* malloc(size_t numBytes)
void* calloc(size_t numElements, size_t elemSize)
void* realloc(void* prevAllocPntr, size_t newSize)
void free(void* allocPntr)
void* aligned_alloc(size_t numBytes, size_t alignmentSize)
*/
