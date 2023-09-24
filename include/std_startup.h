/*
File:   std_startup.h
Author: Taylor Robbins
Date:   09\23\2023
Description: 
	** Holds the mainCRTStartup function. Include this file if you are linking to the standard library dynamically
*/

#ifndef _STD_STARTUP_H
#define _STD_STARTUP_H

#if COMPILER_MSVC
int WinMainCRTStartup()
{
	return main();
}
#elif COMPILER_GCC
int __main()
{
	return 0;
}
int mainCRTStartup()
{
	return main();
}
#else
#error Unsupported compiler in std_startup.c
#endif

#endif //  _STD_STARTUP_H
