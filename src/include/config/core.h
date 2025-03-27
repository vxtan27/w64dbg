// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

// =====================================================================================
//  Debugger Configuration Defaults
// =====================================================================================

#define DEFAULT_TIMEOUT       0      // No timeout
#define DEFAULT_FIRSTBREAK    FALSE  // Ignore first debug break
#define DEFAULT_BREAKPOINT    TRUE   // Enable breakpoints
#define DEFAULT_VERBOSE       FALSE  // Disable verbose output
#define DEFAULT_OUTPUT        TRUE   // Enable standard output
#define DEFAULT_START         FALSE  // Do not auto-start debugging

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
//  Process Creation Flags
// =====================================================================================

#define CREATIONFLAGS  \
    CREATE_UNICODE_ENVIRONMENT | CREATE_BREAKAWAY_FROM_JOB | \
    CREATE_PRESERVE_CODE_AUTHZ_LEVEL | CREATE_DEFAULT_ERROR_MODE

// =====================================================================================
//  Symbol Loading Options
// =====================================================================================

#define SYMOPTIONS  \
    SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | \
    SYMOPT_OMAP_FIND_NEAREST | SYMOPT_NO_UNQUALIFIED_LOADS | \
    SYMOPT_FAIL_CRITICAL_ERRORS | SYMOPT_EXACT_SYMBOLS | \
    SYMOPT_INCLUDE_32BIT_MODULES | SYMOPT_AUTO_PUBLICS | \
    SYMOPT_NO_IMAGE_SEARCH | SYMOPT_NO_PROMPTS | \
    SYMOPT_DISABLE_SYMSRV_AUTODETECT | SYMOPT_SYMPATH_LAST

// =====================================================================================
//  Help Message
// =====================================================================================

#define HELP \
R"(Invalid syntax.
Usage: W64DBG [options] <executable> [exec-args>

Description:
    A native debugging utility for x64 Windows.

Entries:
    options       Options control behavior.
    executable    Target executable file.
    exec-args     Target executable arguments.

Options:
    /B            Ignore breakpoints.
    /O            Suppress OutputDebugString.
    /S            Open in a new console window.
    /T<n>         Wait for input (seconds).
    /V{0|1|2}     Set output verbosity.
)"

// =====================================================================================
//  Error Messages
// =====================================================================================

#define VALUE_EXPECTED      "Value expected for '"
#define _TIMEOUT_INVALID    "Invalid value for timeout ("
#define TIMEOUT_INVALID_    ") specified. Valid range is -1 to 99999.\n"
#define _INVALID_ARGUMENT   "Invalid argument/option - '"
#define INVALID_ARGUMENT_   "'.\n"

// =====================================================================================
//  File Extensions
// =====================================================================================

#define EXTENSION L".exe"  // Executable file extension

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

// =====================================================================================
//  System Debugging Messages
// =====================================================================================

#define _SLE_ERROR       "Invalid data was passed to the function that failed. This caused the application to fail"
#define _SLE_MINORERROR  "Invalid data was passed to the function, but the error probably will not cause the application to fail"
#define _SLE_WARNING     "Potentially invalid data was passed to the function, but the function completed processing"