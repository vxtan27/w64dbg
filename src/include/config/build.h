// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

//------------------------------------------------------------------------------------
// Build Configuration
//------------------------------------------------------------------------------------

#ifndef _DEBUG
    #define NDEBUG // Disable assertions (release)
#endif

#define NO_STRICT         // Disable strict type checking
#define _HAS_EXCEPTIONS 0 // Disable exception handling

//------------------------------------------------------------------------------------
// Feature Exclusions (Optimization)
//------------------------------------------------------------------------------------
// Exclude optional or unused components for smaller binaries and faster builds

#define WIN32_LEAN_AND_MEAN          // Minimize WinAPI headers
#define _CRT_DECLARE_NONSTDC_NAMES 0 // Suppress non-standard CRT names

// User Interface
#define NOWINSTYLES   // Exclude window styles
#define NOSYSMETRICS  // Exclude system metrics
#define NOICONS       // Exclude icons
#define NOKEYSTATES   // Exclude key state flags
#define NOSYSCOMMANDS // Exclude system commands
#define NOSHOWWINDOW  // Exclude ShowWindow API
#define NOCLIPBOARD   // Exclude clipboard API
#define NOCOLOR       // Exclude color definitions
#define NOCTLMGR      // Exclude control management
#define NODRAWTEXT    // Exclude text drawing
#define NOGDI         // Exclude GDI

// System Services & Internationalization
#define NONLS     // Exclude NLS
#define NOMB      // Exclude MessageBox API
#define NOSCROLL  // Exclude scroll bar APIs
#define NOSERVICE // Exclude Service APIs
#define NOWH      // Exclude Windows Hooks

// Windowing & Help
#define NOWINOFFSETS  // Exclude window offsets
#define NOHELP        // Exclude Help API

// Miscellaneous
#define NOMCX // Exclude Modem Configuration
#define NOIME // Exclude IME

// winuser.h Exclusions
#define NODESKTOP       // Exclude Desktop API
#define NOWINDOWSTATION // Exclude Window Station API
#define NOSECURITY      // Exclude Security API
#define NONCMESSAGES    // Exclude non-client messages
#define NOMDI           // Exclude MDI
#define NO_STATE_FLAGS  // Exclude window state flags

// interlockedapi.h Exclusions
#define NOWINBASEINTERLOCK // Exclude base interlock primitives