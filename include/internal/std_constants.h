/*
File:   std_constants.h
Author: Taylor Robbins
Date:   10\07\2023
*/

#ifndef _STD_CONSTANTS_H
#define _STD_CONSTANTS_H

#define WASM_MEMORY_PAGE_SIZE      (64*1024) //64kB or 65,536b
#define WASM_MEMORY_MAX_NUM_PAGES  (64*1024) //65,536 pages * 64 kB/page = 4GB
#define WASM_MEMORY_MAX_SIZE       (WASM_MEMORY_MAX_NUM_PAGES * WASM_MEMORY_PAGE_SIZE)
#define WASM_PROTECTED_SIZE        1024       //1kB at start of wasm memory should be unused and should never be written to

#endif //  _STD_CONSTANTS_H
