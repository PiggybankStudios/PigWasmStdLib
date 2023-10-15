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

typedef struct
{
	char placeholder;
} File_t;

typedef struct
{
	unsigned flags;
	unsigned char* rpos;
	unsigned char* rend;
	int (*close)(File_t*);
	unsigned char* wend;
	unsigned char* wpos;
	unsigned char* mustbezero_1;
	unsigned char* wbase;
	size_t (*read)(File_t*, unsigned char*, size_t);
	size_t (*write)(File_t*, const unsigned char*, size_t);
	off_t (*seek)(File_t*, off_t, int);
	unsigned char* buf;
	size_t buf_size;
	File_t* prev;
	File_t* next;
	int fd;
	int pipe_pid;
	long lockcount;
	int mode;
	volatile int lock;
	int lbf;
	void *cookie;
	off_t off;
	char* getln_buf;
	void* mustbezero_2;
	unsigned char* shend;
	off_t shlim, shcnt;
	File_t* prev_locked;
	File_t* next_locked;
	struct __locale_struct *locale;
} IoFile_t;

void* malloc(size_t numBytes);
void* calloc(size_t numElements, size_t elemSize);
void* realloc(void* prevAllocPntr, size_t newSize);
void free(void* allocPntr);
void* aligned_alloc(size_t numBytes, size_t alignmentSize);

#define RAND_MAX (0x7FFFFFFF)
void srand(unsigned int seed);
int rand();

double strtod(const char* str, char** pntrOut);
double atof(const char* str);
void* alloca(size_t numBytes);

EXTERN_C_END

#endif //  _STDLIB_H

// +--------------------------------------------------------------+
// |                   Autocomplete Dictionary                    |
// +--------------------------------------------------------------+
/*
@Types
IoFile_t
File_t
@Functions
void* malloc(size_t numBytes)
void* calloc(size_t numElements, size_t elemSize)
void* realloc(void* prevAllocPntr, size_t newSize)
void free(void* allocPntr)
void* aligned_alloc(size_t numBytes, size_t alignmentSize)
void srand(unsigned int seed);
int rand();
*/
