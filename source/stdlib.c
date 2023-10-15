/*
File:   stdlib.c
Author: Taylor Robbins
Date:   10\07\2023
Description: 
	** Holds all the implementations for functions declared in stdlib.h
*/

#define PIG_WASM_STD_USE_CHATGPT_ATOF  0

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
	stdRandState = (6364136223846793005ULL * stdRandState) + 1;
	return (stdRandState >> 33);
}

#if !PIG_WASM_STD_USE_CHATGPT_ATOF
// sh_fromstring
// shlim
// __floatscan
// shcnt

#define shcnt(f) ((f)->shcnt + ((f)->rpos - (f)->buf))
#define shlim(f, lim) __shlim((f), (lim))
#define shgetc(f) (((f)->rpos != (f)->shend) ? *(f)->rpos++ : __shgetc(f))
#define shunget(f) ((f)->shlim>=0 ? (void)(f)->rpos-- : (void)0)

#define sh_fromstring(f, s) \
	((f)->buf = (f)->rpos = (void *)(s), (f)->rend = (void*)-1)

#define F_PERM 1
#define F_NORD 4
#define F_NOWR 8
#define F_EOF 16
#define F_ERR 32
#define F_SVB 64
#define F_APP 128

#define EOF (-1)

int __toread(IoFile_t* f)
{
	f->mode |= f->mode-1;
	if (f->wpos != f->wbase) f->write((File_t*)f, 0, 0);
	f->wpos = f->wbase = f->wend = 0;
	if (f->flags & F_NORD) {
		f->flags |= F_ERR;
		return EOF;
	}
	f->rpos = f->rend = f->buf + f->buf_size;
	return (f->flags & F_EOF) ? EOF : 0;
}
int __uflow(IoFile_t* f)
{
	unsigned char c;
	if (!__toread(f) && f->read((File_t*)f, &c, 1)==1) return c;
	return EOF;
}
void __shlim(IoFile_t* f, off_t lim)
{
	f->shlim = lim;
	f->shcnt = f->buf - f->rpos;
	// If lim is nonzero, rend must be a valid pointer.
	if (lim && f->rend - f->rpos > lim) { f->shend = f->rpos + lim; }
	else { f->shend = f->rend; }
}
int __shgetc(IoFile_t* f)
{
	int c;
	off_t cnt = shcnt(f);
	if (f->shlim && cnt >= f->shlim || (c=__uflow(f)) < 0)
	{
		f->shcnt = f->buf - f->rpos + cnt;
		f->shend = f->rpos;
		f->shlim = -1;
		return EOF;
	}
	cnt++;
	if (f->shlim && f->rend - f->rpos > f->shlim - cnt)
	{
		f->shend = f->rpos + (f->shlim - cnt);
	}
	else
	{
		f->shend = f->rend;
	}
	f->shcnt = f->buf - f->rpos + cnt;
	if (f->rpos <= f->buf) f->rpos[-1] = c;
	return c;
}

#include "stdlib_floatscan.c"

static long double strtox(const char* str, char** pntrOut, int prec)
{
	IoFile_t fileHandle;
	sh_fromstring(&fileHandle, str);
	shlim(&fileHandle, 0);
	long double y = __floatscan(&fileHandle, prec, 1);
	off_t count = shcnt(&fileHandle);
	if (pntrOut) { *pntrOut = count ? (char*)str + count : (char*)str; }
	return y;
}
double strtod(const char* str, char** pntrOut)
{
	return strtox(str, pntrOut, 1);
}
#endif
double atof(const char* str)
{
	#if PIG_WASM_STD_USE_CHATGPT_ATOF
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
	#else
	return strtod(str, 0);
	#endif
}

void* alloca(size_t numBytes)
{
	return __builtin_alloca(numBytes);
}