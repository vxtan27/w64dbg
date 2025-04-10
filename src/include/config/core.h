// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

//------------------------------------------------------------------------------------
// Debugger Configuration
//------------------------------------------------------------------------------------

#define DEFAULT_BREAKPOINT   true  // Enable breakpoints by default
#define DEFAULT_VERBOSE      false // Disable verbose output by default
#define DEFAULT_OUTPUT       true  // Enable standard output by default
#define DEFAULT_PAUSE        false // Don't auto-pause on exit by default

//------------------------------------------------------------------------------------
// Timeout Limits
//------------------------------------------------------------------------------------

#define VALID_TIMEOUT   99999  // Maximum valid timeout (milliseconds)
#define INVALID_TIMEOUT 100000 // Value indicating an invalid timeout

//------------------------------------------------------------------------------------
// Debugging Limits
//------------------------------------------------------------------------------------

#define MAX_DLL 32 // Maximum DLLs allowed to be loaded

//------------------------------------------------------------------------------------
// Buffer Sizes (Bytes)
//------------------------------------------------------------------------------------

#define PAGESIZE  4096 // System memory page size
#define BUFLEN    8192 // General-purpose buffer length
#define WBUFLEN   4096 // Wide character buffer length

//------------------------------------------------------------------------------------
// Symbol Loading Options (dbghelp.h)
//------------------------------------------------------------------------------------

#define SYMOPTIONS \
    (SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | \
     SYMOPT_OMAP_FIND_NEAREST | SYMOPT_NO_UNQUALIFIED_LOADS | \
     SYMOPT_FAIL_CRITICAL_ERRORS | SYMOPT_CASE_INSENSITIVE | \
     SYMOPT_INCLUDE_32BIT_MODULES | SYMOPT_AUTO_PUBLICS | SYMOPT_NO_IMAGE_SEARCH | \
     SYMOPT_NO_PROMPTS | SYMOPT_DISABLE_SYMSRV_AUTODETECT)

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

#define CREATE_THREAD   "Thread Created: "
#define CREATE_PROCESS  "Process Created: "
#define LOAD_DLL        "DLL Loaded: "
#define UNLOAD_DLL      "DLL Unloaded: "
#define EXIT_THREAD     "Thread Exited: "
#define EXIT_PROCESS    "Process Exited: "
#define OUTPUT_DEBUG    "Output: "
#define RIP             "RIP Error: "

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