/*
File:   std_startup.c
Author: Taylor Robbins
Date:   09\23\2023
Description: 
	** Holds the mainCRTStartup function. Include this file if you are linking to the standard library dynamically
*/

int WinMainCRTStartup()
{
	return main();
}
