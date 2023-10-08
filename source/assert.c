/*
File:   assert.c
Author: Taylor Robbins
Date:   10\07\2023
Description: 
	** None 
*/

__attribute__((noreturn)) void StdAssertFailure(const char* filePath, int fileLineNum, const char* funcName, const char* conditionStr)
{
	jsAssertFailure(filePath, fileLineNum, funcName, conditionStr);
}
