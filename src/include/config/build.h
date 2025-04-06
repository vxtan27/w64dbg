// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

// =====================================================================================
//  Build Configuration
// =====================================================================================
#ifndef _DEBUG
    #define NDEBUG                   // Disable assertions in release builds
#endif
#define NO_STRICT                    // Relax type checking
#define _HAS_EXCEPTIONS 0            // Disable exceptions
#define WIN32_LEAN_AND_MEAN          // Reduce Windows API inclusions
#define _CRT_DECLARE_NONSTDC_NAMES 0 // Exclude non standard names

// =====================================================================================
//  Windows API Exclusions (Build Optimizations)
// =====================================================================================
// Exclude unused Windows components to reduce bloat and speed up compilation.
#define NOWINSTYLES              // Exclude window styles (WS_*, CS_*, ES_*)
#define NOSYSMETRICS             // Exclude system metrics (SM_*)
#define NOICONS                  // Exclude icon definitions (IDI_*)
#define NOKEYSTATES              // Exclude key state macros (MK_*)
#define NOSYSCOMMANDS            // Exclude system commands (SC_*)
#define NOSHOWWINDOW             // Exclude ShowWindow() and SW_* macros
#define NOCLIPBOARD              // Exclude clipboard functions
#define NOCOLOR                  // Exclude screen color definitions
#define NOCTLMGR                 // Exclude control/dialog routines
#define NODRAWTEXT               // Exclude DrawText() and DT_* macros
#define NOGDI                    // Exclude GDI functions
#define NONLS                    // Exclude National Language Support (NLS)
#define NOMB                     // Exclude MessageBox() and MB_* macros
#define NOSCROLL                 // Exclude scroll bar functions (SB_*)
#define NOSERVICE                // Exclude Windows Service API
#define NOWH                     // Exclude Windows hooks (WH_*)
#define NOWINOFFSETS             // Exclude window offsets (GWL_*, GCL_*)
#define NOHELP                   // Exclude Help API
#define NOMCX                    // Exclude Modem Configuration Extensions
#define NOIME                    // Exclude Input Method Editor (IME)

// =====================================================================================
//  Additional Windows API Exclusions (winuser.h)
// =====================================================================================
#define NODESKTOP                // Exclude Desktop APIs
#define NOWINDOWSTATION          // Exclude Window Station APIs
#define NOSECURITY               // Exclude Security APIs
#define NONCMESSAGES             // Exclude non-client messages
#define NOMDI                    // Exclude Multiple Document Interface (MDI)
#define NO_STATE_FLAGS           // Exclude state flag definitions

// =====================================================================================
//  Windows Base Interlock Exclusions (interlockedapi.h)
// =====================================================================================
#define NOWINBASEINTERLOCK       // Exclude Windows base interlock functions