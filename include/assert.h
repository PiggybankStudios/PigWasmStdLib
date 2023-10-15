/*
File:   assert.h
Author: Taylor Robbins
Date:   10\07\2023
*/

#ifndef _ASSERT_H
#define _ASSERT_H

#include <internal/std_common.h>

EXTERN_C_START

__attribute__((noreturn)) void StdAssertFailure(const char* filePath, int fileLineNum, const char* funcName, const char* conditionStr);

#if STD_ASSERTIONS_ENABLED
#define assert(condition) do { if (!(condition)) { StdAssertFailure(__FILE__, __LINE__, __func__, #condition); } } while(0)
#else
#define	assert(condition) (void)0
#endif

EXTERN_C_END

#endif //  _ASSERT_H

// +--------------------------------------------------------------+
// |                   Autocomplete Dictionary                    |
// +--------------------------------------------------------------+
/*
@Functions
StdAssertFailure(const char* filePath, int fileLineNum, const char* funcName, const char* conditionStr)
#define assert(condition)
*/
