// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

// =====================================================================================
//  CRT & VC Runtime Configuration
// =====================================================================================

#if defined(_DLL)
    #define VC_LIB "vcruntime"       // Dynamic VC runtime (shared)
#else
    #define VC_LIB "libvcruntime"    // Static VC runtime (static)
#endif

// =====================================================================================
//  Library Linking
// =====================================================================================

#pragma comment(lib, VC_LIB)       // Link Microsoft VC++ Runtime
#pragma comment(lib, "ntdll")      // Link Windows Native API
#pragma comment(lib, "kernel32")   // Link Windows Core API
#pragma comment(lib, "dbghelp")    // Link Debugging Tools for Windows