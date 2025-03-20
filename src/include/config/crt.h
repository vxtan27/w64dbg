// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

// Select CRT and VC libraries
#ifdef _DEBUG
    #define CRT_LIB "ucrtd"
    #ifdef _DLL
        #define VC_LIB "vcruntime"
    #else
        #define VC_LIB "libvcruntime"
    #endif
#else
    #ifdef _DLL
        #define CRT_LIB "ucrt"
        #define VC_LIB "vcruntime"
    #else
        #define CRT_LIB "libucrt"
        #define VC_LIB "libvcruntime"
    #endif
#endif

// Link Required Libraries
#pragma comment(lib, CRT_LIB)
#pragma comment(lib, VC_LIB)
#pragma comment(lib, "ntdll")
#pragma comment(lib, "kernel32")
#pragma comment(lib, "dbghelp")