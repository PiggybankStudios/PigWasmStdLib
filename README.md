# Pig WASM Standard Library
This is a very minimal implementation of various C standard library functions and headers that's targeted specifically at compiling to WebAssembly in Clang. This is mainly designed to be used alongside [Gylib](https://github.com/PiggybankStudios/gylib) and [Pig Engine](https://github.com/PiggybankStudios/pigengine) and only has functions and types from the standard library that those repositories rely on.

# Sources
### [Musl Lib-C](https://musl.libc.org/)
### [Sean Barrett's Stb](https://github.com/nothings/stb/)

Many of the implementations are based on functions from [musl libc](https://musl.libc.org/) but almost all have been reformatted and updated to make them a bit more readable (for me at least). Some of the functions have been significantly changed though (like atof) and vsnprintf uses [stb_sprintf.h](https://github.com/nothings/stb/blob/master/stb_sprintf.h). Also the structure of the .c files is majorly redone so it's clearer when dependencies are used by which functions. We also use __builtin\_[func] calls wherever we can to utilize WASM instructions (like memory.fill) or Clang's builtin implementations (like fabs or va_start).

# Imports from Javascript
The implementation of various functions rely on imports from javascript to the WASM module. Here's the full list of imports that we expect (javascript implementations for these functions is provided in std_js_api.js).
```cpp
void jsStdAbort(const char* messageStrPntr);
void jsStdAssertFailure(const char* filePathPntr, int fileLineNum, const char* funcNamePntr, const char* messageStrPntr);
void jsStdGrowMemory(unsigned int numPages);
```

# Functions
Some functions are ommitted for brevity sake. If you want to see the full list, just open one of the header files in the include/ folder
### assert.h
```cpp
void assert(bool condition); //macro
```
### intrin.h
```cpp
void __debugbreak(); //TODO: Currently empty
```
### math.h
```cpp
bool isinf(double/float value); //macro
bool isnan(double/float value); //macro
bool signbit(double/float value); //macro
bool isnormal(double/float value); //macro
utin32_t asuint(float value); //macro
float asfloat(uint32_t value); //macro
utin64_t asuint64(double value); //macro
double asdouble(uint64_t value); //macro
float  fminf(float value1, float value2);      double fmin(double value1, double value2);
float  fmaxf(float value1, float value2);      double fmax(double value1, double value2);
float  fabsf(float value);                     double fabs(double value);
float  fmodf(float numer, float denom);        double fmod(double numer, double denom);
float  roundf(float value);                    double round(double value);
float  floorf(float value);                    double floor(double value);
float  ceilf(float value);                     double ceil(double value);
float  scalbnf(float value, int power);        double scalbn(double value, int power);
float  sinf(float value);                      double sin(double value);
float  asinf(float value);                     double asin(double value);
float  cosf(float value);                      double cos(double value);
float  acosf(float value);                     double acos(double value);
float  tanf(float value);                      double tan(double value);
float  atanf(float value);                     double atan(double value);
float  atan2f(float numer, float denom);       double atan2(double numer, double denom);
float  powf(float value, float exponent);      double pow(double value, double exponent);
float  sqrtf(float value);                     double sqrt(double value);
float  cbrtf(float value);                     double cbrt(double value);
float  logf(float value);                      double log(double value);
float  log2f(float value);                     double log2(double value);
float  log10f(float value);                    double log10(double value);
float  ldexpf(float value, int exponent);      double ldexp(double value, int exponent);
float  expf(float value);                      double exp(double value);
float  copysignf(float magnitude, float sign); double copysign(double magnitude, double sign);
```
### new
```cpp
//Currently empty, just sits here to satisfy an annoying #include
```
### stdarg.h
```cpp
void va_start(va_list args, const char* formatStr); //macro
void va_end(va_list args); //macro
void va_arg(va_list args, type); //macro
void va_copy(va_list dest, va_list source); //macro
```
### stddef.h
```cpp
#define offsetof(type, member)
```
### stdio.h
```cpp
int vsnprintf(char* bufferPntr, size_t bufferSize, const char* formatStr, va_list args);
```
### stdlib.h
```cpp
int abs(int value);
void* malloc(size_t numBytes);
void* calloc(size_t numElements, size_t elemSize); //stub, will assert on call
void* realloc(void* prevAllocPntr, size_t newSize); //stub, will assert on call
void free(void* allocPntr); //stub, will assert on call
void* aligned_alloc(size_t numBytes, size_t alignmentSize); //stub, will assert on call
void srand(unsigned int seed);
int rand();
double atof(const char* str);
void* alloca(size_t numBytes);
void qsort(void* basePntr, size_t numItems, size_t itemSize, CompareFunc_f* compareFunc);
```
### string.h
```cpp
void* memset(void* pntr, int value, size_t numBytes);
int memcmp(const void* left, const void* right, size_t numBytes);
void* memcpy(void* dest, const void* source, size_t numBytes);
void* memmove(void* dest, const void* source, size_t numBytes);
void* strcpy(void* dest, const void* source);
char* strstr(const char* haystack, const char* needle);
int strcmp(const char* left, const char* right);
int strncmp(const char* left, const char* right, size_t numBytes);
size_t strlen(const char* str);
size_t wcslen(const wchar_t* str);
```

# Types
### stdarg.h
```cpp
va_list
```
### stdint.h
```cpp
wchar_t
float_t, double_t
size_t, ssize_t
uintptr_t, ptrdiff_t, intptr_t
int8_t, int16_t, int32_t, int64_t, intmax_t
uint8_t, uint16_t, uint32_t, uint64_t, u_int64_t, uintmax_t
```

# Defines
### float.h
```cpp
FLT_RADIX
FLT_TRUE_MIN
FLT_MIN
FLT_MAX
FLT_EPSILON
FLT_MANT_DIG
FLT_MIN_EXP
FLT_MAX_EXP
FLT_HAS_SUBNORM
FLT_DIG
FLT_DECIMAL_DIG
FLT_MIN_10_EXP
FLT_MAX_10_EXP
DBL_TRUE_MIN
DBL_MIN
DBL_MAX
DBL_EPSILON
DBL_MANT_DIG
DBL_MIN_EXP
DBL_MAX_EXP
DBL_HAS_SUBNORM
DBL_DIG
DBL_DECIMAL_DIG
DBL_MIN_10_EXP
DBL_MAX_10_EXP
DECIMAL_DIG
```
### limits.h
```cpp
CHAR_MIN
CHAR_MAX
CHAR_BIT
SCHAR_MIN
SCHAR_MAX
UCHAR_MAX
SHRT_MIN
SHRT_MAX
USHRT_MAX
INT_MIN
INT_MAX
UINT_MAX
LONG_MIN
LONG_MAX
ULONG_MAX
LLONG_MIN
LLONG_MAX
ULLONG_MAX
```
### stdbool.h
```cpp
true
false
bool
__bool_true_false_are_defined
```
### stddef.h
```cpp
NULL
```
### stdint.h
```cpp
_Addr
_Int64
_Reg
__BYTE_ORDER
__LONG_MAX
INT8_MIN
INT16_MIN
INT32_MIN
INT64_MIN
INT8_MAX
INT16_MAX
INT32_MAX
INT64_MAX
UINT8_MAX
UINT16_MAX
UINT32_MAX
UINT64_MAX
INTMAX_MIN
INTMAX_MAX
UINTMAX_MAX
WCHAR_MAX
WCHAR_MIN
SIG_ATOMIC_MIN
SIG_ATOMIC_MAX
```

# Line and File Counts
```
Folder            Files        Lines         Code     Comments       Blanks
===============================================================================
include/             18         1087          409          489          189
source/              16         6215         4943          838          434
===============================================================================
 Total               34         7302         5352         1327          623
===============================================================================
```
