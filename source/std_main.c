/*
File:   std_main.c
Author: Taylor Robbins
Date:   09\23\2023
Description: 
	** This file includes all other headers .h and .c files in the library
	** It can be compiled to a position independent lib file, or be included directly in your project
*/

#include <internal/std_common.h>

#include "limits.h"
#include "stdint.h"
#include "float.h"
#include "assert.h"
#include "math.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "stdarg.h"
#include "intrin.h"
#include "stdio.h"

#include "std_wasm_memory.c"

#include "assert.c"
#include "stdlib.c"
#include "math.c"
#include "string.c"
#include "intrin.c"
#include "stdio.c"

// +==============================+
// |          InitStdLib          |
// +==============================+
WASM_EXPORTED_FUNC(void, InitStdLib, uint32_t initialMemPageCount)
{
	InitWasmMemory(initialMemPageCount);
}

// +==============================+
// |         GetStackBase         |
// +==============================+
WASM_EXPORTED_FUNC(unsigned int, GetStackBase)
{
	int var1 = 10;
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wreturn-stack-address"
	return (size_t)&var1;
	#pragma clang diagnostic pop
}
