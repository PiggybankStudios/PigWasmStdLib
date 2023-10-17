/*
File:   std_js_api.h
Author: Taylor Robbins
Date:   10\07\2023
Description:
	** Declares imports for all the functions we need to be provided by javascript (in pig_wasm_std_lib.js)
*/

#ifndef _STD_JS_API_H
#define _STD_JS_API_H

WASM_IMPORTED_FUNC _Noreturn void jsStdAbort(const char* messageStrPntr, int exitCode);
WASM_IMPORTED_FUNC _Noreturn void jsStdAssertFailure(const char* filePathPntr, int fileLineNum, const char* funcNamePntr, const char* messageStrPntr);
WASM_IMPORTED_FUNC void jsStdGrowMemory(unsigned int numPages);
WASM_IMPORTED_FUNC unsigned int jsStdGetHeapSize();

#endif //  _STD_JS_API_H

// +--------------------------------------------------------------+
// |                   Autocomplete Dictionary                    |
// +--------------------------------------------------------------+
/*
@Functions
void jsStdAbort(const char* messageStrPntr, int exitCode)
void jsStdAssertFailure(const char* filePathPntr, int fileLineNum, const char* funcNamePntr, const char* messageStrPntr)
void jsStdGrowMemory(unsigned int numPages)
unsigned int jsStdGetHeapSize();
*/
