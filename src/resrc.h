/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

#pragma once

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>

#define W64DBG_FILE_NOT_FOUND "ERROR: The system cannot find the file specified.\n"

// https://learn.microsoft.com/en-us/windows/win32/fileio/file-buffering#alignment-and-file-access-requirements
// Page size is 4,096 bytes on x64 and x86 or 8,192 bytes for Itanium-based systems

#if defined(__ia64__) || defined(_M_IA64)
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
    char bWow64;
    char Console;
    char DataIsLocal;
    char IsFirst;
} USERCONTEXT;