/*
File:   pig_wasm_js_api.h
Author: Taylor Robbins
Date:   10\07\2023
Description:
	** Declares imports for all the functions we need to be provided by javascript (in pig_wasm_std_lib.js)
*/

#ifndef _PIG_WASM_JS_API_H
#define _PIG_WASM_JS_API_H

WASM_IMPORTED_FUNC __attribute__((noreturn)) void jsAbort(const char* messageStrPntr);
WASM_IMPORTED_FUNC __attribute__((noreturn)) void jsAssertFailure(const char* filePathPntr, int fileLineNum, const char* funcNamePntr, const char* messageStrPntr);
WASM_IMPORTED_FUNC void jsGrowMemory(unsigned int numPages);
WASM_IMPORTED_FUNC void jsTestFunction();
WASM_IMPORTED_FUNC void jsPrintNumber(const char* labelStrPntr, int number);
WASM_IMPORTED_FUNC void jsPrintFloat(const char* labelStrPntr, double number);
WASM_IMPORTED_FUNC void jsPrintString(const char* labelStrPntr, const char* strPntr);

#endif //  _PIG_WASM_JS_API_H
