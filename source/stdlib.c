/*
File:   stdlib.c
Author: Taylor Robbins
Date:   10\07\2023
Description: 
	** Holds all the implementations stdlib.h declared functions
*/

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
