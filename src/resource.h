#pragma once

#define ANSI
#define _ANSI
#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>

#ifdef _MSC_VER
#define __FORCEINLINE __forceinline
#else
#define __FORCEINLINE inline __attribute__((always_inline))
#endif

#define W64DBG_ERROR_INVALID_TIMEOUT "ERROR: Invalid value for timeout (/T) specified. Valid range is -1 to 99999.\n"
#define W64DBG_ERROR_INVALID W64DBG_ERROR_INVALID_TIMEOUT
#define W64DBG_FILE_NOT_FOUND "The system cannot find the file specified.\n"
#define W64DBG_UNKNOWN "\x1b[m ("
#define W64DBG_IN "\x1b[m in \x1b[33m"

// https://learn.microsoft.com/en-us/windows/win32/fileio/file-buffering#alignment-and-file-access-requirements
//Page size is 4,096 bytes on x64 and x86 or 8,192 bytes for Itanium-based systems

#if defined(__ia64__) || defined(_M_IA64)
#define PAGESIZE 8192
#else
#define PAGESIZE 4096
#endif

#define BUFLEN PAGESIZE

typedef struct
{
    HANDLE hProcess;
    DWORD64 Offset;
    char *p;
    char Console;
    char IsFirst;
} USERCONTEXT;