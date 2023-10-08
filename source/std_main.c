/*
File:   std_main.c
Author: Taylor Robbins
Date:   09\23\2023
Description: 
	** This file includes all other headers .h and .c files in the library
	** It can be compiled to a position independent lib file, or be included directly in your project
*/

#include "std_common.h"

#include "stdint.h"
#include "float.h"
#include "assert.h"
#include "math.h"
#include "stdlib.h"

#include "std_wasm_memory.c"

#include "assert.c"
#include "stdlib.c"
#include "math.c"

// +==============================+
// |      InitPigWasmStdLib       |
// +==============================+
WASM_EXPORTED_FUNC(void, InitStdLib, uint32_t initialMemPageCount)
{
	InitWasmMemory(initialMemPageCount);
}
