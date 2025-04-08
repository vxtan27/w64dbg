// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

// =====================================================================================
//  Debugger Configuration Defaults
// =====================================================================================

#define DEFAULT_BREAKPOINT    TRUE   // Enable breakpoints
#define DEFAULT_VERBOSE       FALSE  // Disable verbose output
#define DEFAULT_OUTPUT        TRUE   // Enable standard output
#define DEFAULT_PAUSE         FALSE  // Do not auto-pause before exit

// =====================================================================================
//  Timeout Constraints
// =====================================================================================

#define VALID_TIMEOUT         99999  // Maximum valid timeout value
#define INVALID_TIMEOUT       100000 // Invalid timeout value beyond limit

// =====================================================================================
//  Debugging Constraints
// =====================================================================================

#define MAX_DLL               16     // Maximum DLLs loaded at runtime

// =====================================================================================
//  Buffer Sizes
// =====================================================================================

#define PAGESIZE    4096             // Memory page size
#define BUFLEN      8192             // General buffer length
#define WBUFLEN     4096             // Wide character buffer length


// =====================================================================================
//  Symbol Loading Options
// =====================================================================================

#define SYMOPTIONS  \
    SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | \
    SYMOPT_OMAP_FIND_NEAREST | SYMOPT_NO_UNQUALIFIED_LOADS | \
    SYMOPT_FAIL_CRITICAL_ERRORS | SYMOPT_CASE_INSENSITIVE | \
    SYMOPT_INCLUDE_32BIT_MODULES | SYMOPT_AUTO_PUBLICS | SYMOPT_NO_IMAGE_SEARCH | \
    SYMOPT_NO_PROMPTS | SYMOPT_DISABLE_SYMSRV_AUTODETECT

// =====================================================================================
//  Help Message
// =====================================================================================

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

// =====================================================================================
//  Error Messages
// =====================================================================================

#define _INVALID_ARGUMENT   "ERROR: Invalid argument/option - '"
#define INVALID_ARGUMENT_   "'.\nType \"W64DBG /?\" for usage.\n"

// =====================================================================================
//  Verbose Messages
// =====================================================================================

#define CREATE_THREAD   "CreateThread "
#define CREATE_PROCESS  "CreateProcess "
#define LOAD_DLL        "LoadDll "
#define UNLOAD_DLL      "UnloadDll "
#define EXIT_THREAD     "ExitThread "
#define EXIT_PROCESS    "ExitProcess "
#define OUTPUT_DEBUG    "OutputDebugString "
#define RIP            "Rip "

// =====================================================================================
//  Console Formatting
// =====================================================================================

#define CONSOLE_DEFAULT_FORMAT  "\x1b[m"
#define CONSOLE_NRED_FORMAT     "\x1b[31m"
#define CONSOLE_BLUE_FORMAT     "\x1b[34m"
#define CONSOLE_GREEN_FORMAT    "\x1b[32m"

// =====================================================================================
//  Exception Messages
// =====================================================================================

#define THREAD_NUMBER   "Thread "
#define THREAD_RAISED   " raised exception 0x"
#define EXCEPTION_IN    "\x1b[m in \x1b[33m"
#define EXCEPTION_AT    "at "
#define EXCEPTION_FROM  "from "