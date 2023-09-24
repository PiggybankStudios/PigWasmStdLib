/*
File:   std_os_defines.h
Author: Taylor Robbins
Date:   09\23\2023
*/

#ifndef _STD_OS_DEFINES_H
#define _STD_OS_DEFINES_H

// +--------------------------------------------------------------+
// |    OS_WINDOWS / OS_LINUX / OS_OSX / OS_PLAYDATE / OS_ORCA    |
// +--------------------------------------------------------------+
#if !defined(OS_WINDOWS) && !defined(OS_LINUX) && !defined(OS_OSX) && !defined(OS_PLAYDATE) && !defined(OS_ORCA)
#error You must define OS_WINDOWS, OS_LINUX, OS_OSX, OS_PLAYDATE or OS_ORCA
#endif

#ifdef OS_WINDOWS
#undef OS_WINDOWS
#define OS_WINDOWS 1
#else
#define OS_WINDOWS 0
#endif

#ifdef OS_LINUX
#undef OS_LINUX
#define OS_LINUX 1
#else
#define OS_LINUX 0
#endif

#ifdef OS_OSX
#undef OS_OSX
#define OS_OSX 1
#else
#define OS_OSX 0
#endif

#ifdef OS_PLAYDATE
#undef OS_PLAYDATE
#define OS_PLAYDATE 1
#else
#define OS_PLAYDATE 0
#endif

#ifdef OS_ORCA
#undef OS_ORCA
#define OS_ORCA 1
#else
#define OS_ORCA 0
#endif

#endif //  _STD_OS_DEFINES_H

// +--------------------------------------------------------------+
// |                   Autocomplete Dictionary                    |
// +--------------------------------------------------------------+
/*
@Defines
OS_WINDOWS
OS_LINUX
OS_OSX
OS_PLAYDATE
OS_ORCA
*/
