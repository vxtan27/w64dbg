/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// https://learn.microsoft.com/en-us/windows/win32/fileio/file-buffering#alignment-and-file-access-requirements
// Page size is 4,096 bytes on x64 and x86 or 8,192 bytes for Itanium-based systems

#if defined(_M_IA64) || defined(__ia64__)
#define PAGESIZE 8192
#else
#define PAGESIZE 4096
#endif

#define BUFLEN (PAGESIZE << 1)
#define WBUFLEN (BUFLEN >> 1)

typedef struct
{
    HANDLE hProcess;
    PVOID pContext;
    char *p;
    DWORD bx64win;
    char Console;
    char DataIsLocal;
    char IsFirst;
} USERCONTEXT;