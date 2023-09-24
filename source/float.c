/*
File:   float.c
Author: Taylor Robbins
Date:   09\23\2023
Description: 
	** Holds all the implementations for float.h declared functions
*/

#if COMPILER_MSVC
//The MSVC compiler generates a requirement for this symbol the moment floating point numbers are used
EXTERN_C_START
void _fltused() { }
EXTERN_C_END
#endif
