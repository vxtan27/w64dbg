// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

// =====================================================================================
//  Build Configuration
// =====================================================================================

#define NO_STRICT                // Disable strict type checking
#define WIN32_LEAN_AND_MEAN      // Exclude rarely used Windows APIs
#define DBG_DEBUG_SINGLE_PROCESS // Optimize for single-process debugging

// =====================================================================================
//  Windows API Exclusions (Optimize Build)
// =====================================================================================
//  Minimize unnecessary Windows headers to speed up compilation

// windows.h
#define NOWINSTYLES              // Exclude window styles (WS_*, CS_*, ES_*, etc)
#define NOSYSMETRICS             // Exclude system metrics (SM_*)
#define NOICONS                  // Exclude icon definitions (IDI_*)
#define NOKEYSTATES              // Exclude key state macros (MK_*)
#define NOSYSCOMMANDS            // Exclude system commands (SC_*)
#define NOSHOWWINDOW             // Exclude ShowWindow() and SW_* macros
#define NOCLIPBOARD              // Exclude clipboard functions
#define NOCOLOR                  // Exclude screen color definitions
#define NOCTLMGR                 // Exclude control and dialog routines
#define NODRAWTEXT               // Exclude DrawText() and DT_* macros
#define NOGDI                    // Exclude GDI functions and definitions
#define NONLS                    // Exclude National Language Support (NLS)
#define NOMB                     // Exclude MessageBox() and MB_* macros
#define NOSCROLL                 // Exclude scroll bar functions (SB_*)
#define NOSERVICE                // Exclude Windows Service API
#define NOWH                     // Exclude Windows hooks (WH_*)
#define NOWINOFFSETS             // Exclude window offsets (GWL_*, GCL_*)
#define NOHELP                   // Exclude Help engine API
#define NOMCX                    // Exclude Modem Configuration Extensions
#define NOIME                    // Exclude Input Method Editor (IME)

// winuser.h
#define NODESKTOP                // Exclude Desktop APIs
#define NOWINDOWSTATION          // Exclude Window Station APIs
#define NOSECURITY               // Exclude Security APIs
#define NONCMESSAGES             // Exclude non-client messages
#define NOMDI                    // Exclude Multiple Document Interface (MDI)
#define NO_STATE_FLAGS           // Exclude state flag definitions

// interlockedapi.h
#define NOWINBASEINTERLOCK       // Exclude Windows base interlock functions