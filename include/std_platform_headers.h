/*
File:   std_platform_headers.h
Author: Taylor Robbins
Date:   10\07\2023
*/

#ifndef _STD_PLATFORM_HEADERS_H
#define _STD_PLATFORM_HEADERS_H

#if ARCH_WASM
//TODO: What do we include in this case?
#elif OS_WINDOWS
//Usually found in C:\Program Files (x86)\Windows Kits\10\Include\[version_number]\um\Windows.h
// #include <Windows.h>
#endif

#endif //  _STD_PLATFORM_HEADERS_H
