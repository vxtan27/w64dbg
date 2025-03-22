// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

// =========================================================================================
//  Compilation Settings
// =========================================================================================

#define NO_STRICT                 // Disable strict type checking
#define WIN32_LEAN_AND_MEAN       // Exclude rarely used APIs

// =========================================================================================
//  Windows API Exclusions
// =========================================================================================

// Windows.h
#define NOGDICAPMASKS             // CC_*, LC_*, PC_*, CP_*, TC_*, RC_
#define NOWINSTYLES               // WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
#define NOSYSMETRICS              // SM_*
#define NOICONS                   // IDI_*
#define NOKEYSTATES               // MK_*
#define NOSYSCOMMANDS             // SC_*
#define NORASTEROPS               // Binary and Tertiary raster ops
#define NOSHOWWINDOW              // SW_*
#define OEMRESOURCE               // OEM Resource values
#define NOATOM                    // Atom Manager routines
#define NOCLIPBOARD               // Clipboard routines
#define NOCOLOR                   // Screen colors
#define NOCTLMGR                  // Control and Dialog routines
#define NODRAWTEXT                // DrawText() and DT_*
#define NOGDI                     // All GDI defines and routines
#define NOKERNEL                  // All KERNEL defines and routines
#define NONLS                     // All NLS defines and routines
#define NOMB                      // MB_* and MessageBox()
#define NOMEMMGR                  // GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMETAFILE                // typedef METAFILEPICT
#define NOMINMAX                  // Macros min(a,b) and max(a,b)
#define NOOPENFILE                // OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOSCROLL                  // SB_* and scrolling routines
#define NOSERVICE                 // All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND                   // Sound driver routines
#define NOTEXTMETRIC              // typedef TEXTMETRIC and associated routines
#define NOWH                      // SetWindowsHook and WH_*
#define NOWINOFFSETS              // GWL_*, GCL_*, associated routines
#define NOCOMM                    // COMM driver routines
#define NOKANJI                   // Kanji support stuff.
#define NOHELP                    // Help engine interface.
#define NOPROFILER                // Profiler interface.
#define NODEFERWINDOWPOS          // DeferWindowPos routines
#define NOMCX                     // Modem Configuration Extensions
#define NOCRYPT                   // Cryptographic APIs.
#define NOIME                     // Input Method Manager.

// WinUser.h
#define NODESKTOP                 // Desktop APIs
#define NOWINDOWSTATION           // Window Station APIs
#define NOSECURITY                // Security APIs
#define NONCMESSAGES              // Non-client messages
#define NOMDI                     // Multiple Document Interface
#define NO_STATE_FLAGS            // State flag definitions

// interlockedapi.h
#define NOWINBASEINTERLOCK        // Windows base interlock functions