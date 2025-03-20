// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

// Debugger Configuration Defaults
#define DEFAULT_BREAKPOINT    TRUE
#define DEFAULT_FIRSTBREAK    FALSE
#define DEFAULT_TIMEOUT       0
#define DEFAULT_VERBOSE       FALSE
#define DEFAULT_OUTPUT        TRUE
#define DEFAULT_START         FALSE

// Timeout Constraints
#define VALID_TIMEOUT         99999
#define INVALID_TIMEOUT       100000

// Debugging Constraints
#define MAX_THREAD            32
#define MAX_DLL               16
#define LATENCY               25

// Buffer Sizes
#define PAGESIZE    4096
#define BUFLEN      8192
#define WBUFLEN     4096

// Process Creation Flags
#define CREATIONFLAGS \
    CREATE_UNICODE_ENVIRONMENT | \
    CREATE_BREAKAWAY_FROM_JOB | \
    CREATE_PRESERVE_CODE_AUTHZ_LEVEL | \
    CREATE_DEFAULT_ERROR_MODE

// Symbol Loading Options
#define SYMOPTIONS \
    SYMOPT_UNDNAME | \
    SYMOPT_DEFERRED_LOADS | \
    SYMOPT_LOAD_LINES | \
    SYMOPT_OMAP_FIND_NEAREST | \
    SYMOPT_NO_UNQUALIFIED_LOADS | \
    SYMOPT_FAIL_CRITICAL_ERRORS | \
    SYMOPT_EXACT_SYMBOLS | \
    SYMOPT_INCLUDE_32BIT_MODULES | \
    SYMOPT_AUTO_PUBLICS | \
    SYMOPT_NO_IMAGE_SEARCH | \
    SYMOPT_NO_PROMPTS | \
    SYMOPT_DISABLE_SYMSRV_AUTODETECT | \
    SYMOPT_SYMPATH_LAST

// Help Message
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

// Error Messages
#define VALUE_EXPECTED "Value expected for '"
#define _TIMEOUT_INVALID "Invalid value for timeout ("
#define TIMEOUT_INVALID_ ") specified. Valid range is -1 to 99999.\n"
#define _INVALID_ARGUMENT "Invalid argument/option - '"
#define INVALID_ARGUMENT_ "'.\n"

// Environment Variables
#define PATHENV L"PATH"
#define TMPENV L"TMP"

// File Extensions
#define EXTENSION L".exe"

// Verbose Messages
#define CREATE_THREAD "CreateThread "
#define CREATE_PROCESS "CreateProcess "
#define LOAD_DLL "LoadDll "
#define UNLOAD_DLL "UnloadDll "
#define EXIT_THREAD "ExitThread "
#define EXIT_PROCESS "ExitProcess "
#define OUTPUT_DEBUG "OutputDebugString "
#define RIP "Rip "

// Console Formatting
#define CONSOLE_DEFAULT_FORMAT "\x1b[m"
#define CONSOLE_NRED_FORMAT "\x1b[31m"
#define CONSOLE_BLUE_FORMAT "\x1b[34m"
#define CONSOLE_GREEN_FORMAT "\x1b[32m"

// Object Manager Namespace
#define OBJECT_MANAGER_NAMESPACE (GDB_COMMAND_LINE + 14)
#define OBJECT_MANAGER_NAMESPACE_WLEN wcslen(OBJECT_MANAGER_NAMESPACE)
#define OBJECT_MANAGER_NAMESPACE_LEN (OBJECT_MANAGER_NAMESPACE_WLEN << 1)

// File Names
#define GDB_EXE L"gdb.exe"
#define W64DBG L"w64dbg"

// GDB Command Line
#define GDB_BATCH L" --batch"
#define GDB_COMMAND_LINE L"gdb.exe -q -x=\\??\\"

// GDB Commands
#define GDB_COMMAND_0 "set bac l 100\n" // set backtrace limit 100
#define GDB_COMMAND_1 "set con of\n" // set confirm off
#define GDB_COMMAND_2 "set p th of\n" // set print thread-events off
#define GDB_COMMAND_3 "set p i of\n" // set print inferior-events off
#define GDB_COMMAND_4 "set p en n\n" // set print entry-values no
#define GDB_COMMAND_5 "set lo f NUL\n" // set logging file
#define GDB_COMMAND_6 "set lo r\n" // set logging redirect
#define GDB_COMMAND_7 "set lo d\n" // set logging debugredirect
#define GDB_COMMAND_8 "set lo e\n" // set logging enabled
#define GDB_COMMAND_9 "at " // attach

// GDB Defaults
#define GDB_DEFAULT \
    GDB_COMMAND_0 \
    GDB_COMMAND_1 \
    GDB_COMMAND_2 \
    GDB_COMMAND_3 \
    GDB_COMMAND_4 \
    GDB_COMMAND_5 \
    GDB_COMMAND_6 \
    GDB_COMMAND_7 \
    GDB_COMMAND_8 \
    GDB_COMMAND_9 \

// GDB Commands
#define GDB_PRESERVE "\nset pa of\nset wi 0\n" // set pagination off & set width 0
#define GDB_STYLE "\nset sty e\n" // set style enabled
#define GDB_FRAME_ARG "set p frame-a a\nset p frame-i source-a\nset p pr\n" // set print frame-arguments all & set print frame-info source-and-location & set print pretty
#define GDB_CONTINUE "c\nset lo e of\nbt" // continue & set logging enabled off & backtrace
#define GDB_QUIT "q\n" // quit


// Exception Messages
#define THREAD_NUMBER "Thread #"
#define THREAD_TRIGGERD " triggered exception 0x"
#define EXCEPTION_IN "\x1b[m in \x1b[33m"
#define EXCEPTION_AT "at "
#define EXCEPTION_FROM "from "

// System Debugging Messages
#define _SLE_ERROR "Invalid data was passed to the function that failed. This caused the application to fail"
#define _SLE_MINORERROR "Invalid data was passed to the function, but the error probably will not cause the application to fail"
#define _SLE_WARNING "Potentially invalid data was passed to the function, but the function completed processing"