/*
File:   assert.c
Author: Taylor Robbins
Date:   10\07\2023
Description: 
	** Holds all the implementations for functions declared in assert.h
*/

__attribute__((noreturn)) void StdAssertFailure(const char* filePath, int fileLineNum, const char* funcName, const char* conditionStr)
{
	jsStdAssertFailure(filePath, fileLineNum, funcName, conditionStr);
}
