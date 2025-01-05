/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

// Debugger's default configuration
#define DEFAULT_TIMEOUT       0
#define DEFAULT_DEBUG         FALSE
#define DEFAULT_BREAKPOINT    TRUE
#define DEFAULT_FIRSTBREAK    FALSE
#define DEFAULT_OUTPUT        TRUE
#define DEFAULT_VERBOSE       FALSE
#define DEFAULT_START         FALSE
#define DEFAULT_HELP          FALSE
#define DEFAULT_LEN           125

// Debugger buffer length
#define PAGESIZE    4096
#define BUFLEN      8192
#define WBUFLEN     4096

// Returns all attributes to the default state prior to modification
static const char CONSOLE_DEFAULT_FORMAT[3] = "\x1b[m";

// Faster Builds
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <wchar.h>
#include <stdio.h>
#include <windows.h>
#include <dbghelp.h>
#include <psapi.h>

#include "legal.h"
#include "ntdll.h"
#include "string\conversion.h"
#include "string\format.h"
#include "symbols.h"
#include "timeout.h"

#define MAX_THREAD 32
#define MAX_DLL 16
#define MINGW 2
#define LATENCY 25

#define CreationFlags \
    CREATE_BREAKAWAY_FROM_JOB | \
    CREATE_DEFAULT_ERROR_MODE | \
    CREATE_PRESERVE_CODE_AUTHZ_LEVEL | \
    INHERIT_PARENT_AFFINITY

#define SymOptions \
    SYMOPT_DEFERRED_LOADS | \
    SYMOPT_INCLUDE_32BIT_MODULES | \
    SYMOPT_NO_PUBLICS | \
    SYMOPT_NO_UNQUALIFIED_LOADS

#define NDebugSymOptions \
    SymOptions | \
    SYMOPT_NO_IMAGE_SEARCH | \
    SYMOPT_DISABLE_SYMSRV_AUTODETECT | \
    SYMOPT_IGNORE_NT_SYMPATH

#define _DebugSymOptions \
    SymOptions | \
    SYMOPT_AUTO_PUBLICS | \
    SYMOPT_LOAD_ANYTHING | \
    SYMOPT_LOAD_LINES

static const char HELP[589] =
"Invalid syntax.\n"
"Usage: W64DBG [options] <executable> [exec-args]\n"
"\n"
"Description:\n"
"    A native debugging utility for x64 Windows.\n"
"\n"
"Entries:\n"
"    options       Options control behavior.\n"
"    executable    Target executable file.\n"
"    exec-args     Target executable arguments.\n"
"\n"
"Options:\n"
"    /B<n>         Set breakpoints ignorance.\n"
"    /D            Load PDB debug symbols.\n"
"    /G[+]         Load DWARF debug symbols.\n"
"    /O            Suppress OutputDebugString.\n"
"    /S            Open in a new console window.\n"
"    /T<n>         Wait for input (seconds).\n"
"    /V<n>         Set output verbosity.\n";

static const char VALUE_EXPECTED[20] =
"Value expected for '";
static const char INVALID_TIMEOUT[70] =
"Invalid value for timeout (  ) specified. Valid range is -1 to 99999.\n";
static const char _INVALID_ARGUMENT[27] = "Invalid argument/option - '";
static const char INVALID_ARGUMENT_[3] = "'.\n";

static const char _SLE_ERROR[88] =
"Invalid data was passed to the function that failed. This caused the application to fail";
static const char _SLE_MINORERROR[102] =
"Invalid data was passed to the function, but the error probably will not cause the application to fail";
static const char _SLE_WARNING[90] =
"Potentially invalid data was passed to the function, but the function completed processing";