// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

//------------------------------------------------------------------------------------
// Debugger Configuration
//------------------------------------------------------------------------------------

#define DEFAULT_BREAKPOINT   TRUE  // Enable breakpoints by default
#define DEFAULT_VERBOSE      FALSE // Disable verbose output by default
#define DEFAULT_OUTPUT       TRUE  // Enable standard output by default
#define DEFAULT_PAUSE        FALSE // Don't auto-pause on exit by default

//------------------------------------------------------------------------------------
// Timeout Limits
//------------------------------------------------------------------------------------

#define VALID_TIMEOUT   99999  // Max valid timeout (ms)
#define INVALID_TIMEOUT 100000 // Invalid timeout threshold (ms)

//------------------------------------------------------------------------------------
// Debugging Limits
//------------------------------------------------------------------------------------

#define MAX_DLL 32 // Max DLLs allowed for loading

//------------------------------------------------------------------------------------
// Buffer Sizes
//------------------------------------------------------------------------------------

#define BUFLEN    PAGE_SIZE        // General buffer size
#define WBUFLEN   (PAGE_SIZE >> 1) // Wide char buffer size

//------------------------------------------------------------------------------------
// Symbol Options
//------------------------------------------------------------------------------------

#define SYM_OPTIONS (SYMOPT_DEFERRED_LOADS        | \
                     SYMOPT_LOAD_LINES            | \
                     SYMOPT_UNDNAME               | \
                     SYMOPT_AUTO_PUBLICS          | \
                     SYMOPT_FAVOR_COMPRESSED      | \
                     SYMOPT_OMAP_FIND_NEAREST     | \
                     SYMOPT_INCLUDE_32BIT_MODULES | \
                     SYMOPT_NO_UNQUALIFIED_LOADS  | \
                     SYMOPT_NO_IMAGE_SEARCH       | \
                     SYMOPT_NO_PROMPTS            | \
                     SYMOPT_FAIL_CRITICAL_ERRORS  | \
                     SYMOPT_READONLY_CACHE        )

//------------------------------------------------------------------------------------
// Help Message
//------------------------------------------------------------------------------------

#define HELP \
R"(ERROR: Invalid syntax.
Usage: W64DBG [options] <executable> [exec-args]

Description:
    A native debugging utility for x64 Windows.

Entries:
    options       Options control behavior.
    executable    Target executable file.
    exec-args     Target executable arguments.

Options:
    /B            Ignore breakpoints.
    /O            Suppress OutputDebugString.
    /T            Wait for a key press to exit.
    /V{0|1|2}     Set output verbosity.
    /?            Display this help message.
)"

//------------------------------------------------------------------------------------
// Error Messages
//------------------------------------------------------------------------------------

#define _INVALID_ARGUMENT  "ERROR: Invalid argument/option - '"
#define INVALID_ARGUMENT_  "'.\nUsage: W64DBG /?\n"

//------------------------------------------------------------------------------------
// Verbose Output Prefixes
//------------------------------------------------------------------------------------

#define CREATE_THREAD   "CreateThread "
#define CREATE_PROCESS  "CreateProcess "
#define LOAD_DLL        "LoadDll "
#define UNLOAD_DLL      "UnloadDll "
#define EXIT_THREAD     "ExitThread "
#define EXIT_PROCESS    "ExitProcess "
#define OUTPUT_DEBUG    "OutputDebugString "
#define RIP             "Rip "

//------------------------------------------------------------------------------------
// Console Formatting (ANSI Escape Codes)
//------------------------------------------------------------------------------------

#define CONSOLE_DEFAULT_FORMAT "\x1b[m"   // Reset formatting
#define CONSOLE_NRED_FORMAT    "\x1b[31m"  // Normal Red
#define CONSOLE_BLUE_FORMAT    "\x1b[34m"  // Blue
#define CONSOLE_GREEN_FORMAT   "\x1b[32m"  // Green

//------------------------------------------------------------------------------------
// Exception Reporting
//------------------------------------------------------------------------------------

#define THREAD_NUMBER  "Thread "
#define THREAD_RAISED  " raised exception 0x"
#define EXCEPTION_IN   "\x1b[m in \x1b[33m" // Yellow
#define EXCEPTION_AT   " at "
#define EXCEPTION_FROM " from "