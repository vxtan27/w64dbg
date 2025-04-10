// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

//-------------------------------------------------------------------------------------
// VC Runtime Configuration
//-------------------------------------------------------------------------------------

#ifdef _DEBUG
    #define CRT_LIB "ucrtd.lib"   // Debug UCRT
    #ifdef _DLL
        #define VC_LIB "vcruntime.lib" // Dynamic VC runtime
    #else
        #define VC_LIB "libvcruntime.lib" // Static VC runtime
    #endif
#else
    #ifdef _DLL
        #define CRT_LIB "ucrt.lib"     // Release UCRT (shared)
        #define VC_LIB "vcruntime.lib" // Dynamic VC runtime
    #else
        #define CRT_LIB "libucrt.lib"  // Release UCRT (static)
        #define VC_LIB "libvcruntime.lib" // Static VC runtime
    #endif
#endif

//-------------------------------------------------------------------------------------
// Library Linking
//-------------------------------------------------------------------------------------

#pragma comment(lib, "ntdll.lib")      // Windows Native API
#pragma comment(lib, "kernelbase.lib") // Windows Base API
#pragma comment(lib, "kernel32.lib")   // Windows Kernel API
#pragma comment(lib, CRT_LIB)          // Universal C Runtime
#pragma comment(lib, VC_LIB)           // MSVC Runtime
#pragma comment(lib, "dbghelp.lib")    // Debugging Helper