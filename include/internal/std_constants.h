/*
File:   std_constants.h
Author: Taylor Robbins
Date:   10\07\2023
*/

#ifndef _STD_CONSTANTS_H
#define _STD_CONSTANTS_H

#define WASM_MEMORY_PAGE_SIZE      (64*1024)  //64kB or 65,536 bytes
#define WASM_MEMORY_MAX_NUM_PAGES  (512*1024) //2GB or 524,288 pages
#define WASM_PROTECTED_SIZE        1024       //1kB at start of wasm memory should be unused and should never be written to

#endif //  _STD_CONSTANTS_H
