// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

// =====================================================================================
//  CRT & VC Runtime Configuration
// =====================================================================================

#if defined(_DEBUG)
    #define CRT_LIB "ucrtd"              // Debug Universal C Runtime
    #if defined(_DLL)
        #define VC_LIB "vcruntime"       // Shared VC runtime (dynamic)
    #else
        #define VC_LIB "libvcruntime"    // Static VC runtime (static)
    #endif
#else
    #if defined(_DLL)
        #define CRT_LIB "ucrt"           // Release Universal C Runtime (shared)
        #define VC_LIB "vcruntime"       // Shared VC runtime (dynamic)
    #else
        #define CRT_LIB "libucrt"        // Release Universal C Runtime (static)
        #define VC_LIB "libvcruntime"    // Static VC runtime (static)
    #endif
#endif

// =====================================================================================
//  Library Linking
// =====================================================================================

#pragma comment(lib, CRT_LIB)      // Link Universal C Runtime
#pragma comment(lib, VC_LIB)       // Link Microsoft Visual C++ Runtime
#pragma comment(lib, "ntdll")      // Link Windows Native API
#pragma comment(lib, "kernel32")   // Link Windows Core API
#pragma comment(lib, "dbghelp")    // Link Debugging Tools for Windows