/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

// Debugger's default configuration
#define DEFAULT_DEBUG_DWARF   FALSE
#define DEFAULT_BREAKPOINT    TRUE
#define DEFAULT_FIRSTBREAK    FALSE
#define DEFAULT_TIMEOUT       0
#define DEFAULT_VERBOSE       FALSE
#define DEFAULT_OUTPUT        TRUE
#define DEFAULT_START         FALSE
#define DEFAULT_HELP          FALSE

#define VALID_TIMEOUT 99999
#define INVALID_TIMEOUT (VALID_TIMEOUT + 1)

#define MAX_THREAD 32
#define MAX_DLL 16
#define MINGW_KEEP 1
#define MINGW_NOKEEP 2
#define LATENCY 25

// Debugger buffer length
#define PAGESIZE    4096
#define BUFLEN      8192
#define WBUFLEN     4096

// Faster Builds
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <wchar.h>
#include <windows.h>
#include <devioctl.h>
#include <dbghelp.h>
#include <psapi.h>

// GDB command line arguments
static const wchar_t GDB_COMMAND_LINE[18] = L"gdb.exe -q -x=\\??\\";
// Returns all attributes to the default state prior to modification
static const char CONSOLE_DEFAULT_FORMAT[3] = "\x1b[m";

// static const wchar_t OBJECT_MANAGER_NAMESPACE[4] = L"\\??\\";
// static const char OBJECT_MANAGER_NAMESPACE[] = "\\\0?\0?\0\\";

#define OBJECT_MANAGER_NAMESPACE GDB_COMMAND_LINE + 14
#define OBJECT_MANAGER_NAMESPACE_LEN 8
#define OBJECT_MANAGER_NAMESPACE_WLEN (OBJECT_MANAGER_NAMESPACE_LEN >> 1)

#include "ntdll.h"
#include "string\conversion.h"
#include "string\wmemchr.h"
#include "string\format.h"
#include "symbols.h"
#include "timeout.h"

#define CreationFlags \
    CREATE_UNICODE_ENVIRONMENT | \
    CREATE_BREAKAWAY_FROM_JOB | \
    CREATE_PRESERVE_CODE_AUTHZ_LEVEL | \
    CREATE_DEFAULT_ERROR_MODE

#define SymOptions \
    SYMOPT_UNDNAME | \
    SYMOPT_DEFERRED_LOADS | \
    SYMOPT_LOAD_LINES | \
    SYMOPT_OMAP_FIND_NEAREST | \
    SYMOPT_NO_UNQUALIFIED_LOADS | \
    SYMOPT_FAIL_CRITICAL_ERRORS | \
    SYMOPT_EXACT_SYMBOLS | \
    SYMOPT_AUTO_PUBLICS | \
    SYMOPT_NO_IMAGE_SEARCH | \
    SYMOPT_NO_PROMPTS | \
    SYMOPT_DISABLE_SYMSRV_AUTODETECT | \
    SYMOPT_SYMPATH_LAST

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
static const char TIMEOUT_INVALID[70] =
"Invalid value for timeout (  ) specified. Valid range is -1 to 99999.\n";
static const char _INVALID_ARGUMENT[27] = "Invalid argument/option - '";
static const char INVALID_ARGUMENT_[3] = "'.\n";

static const wchar_t PATHENV[4] = L"PATH";
static const wchar_t EXTENSION[] = L".exe";

static const char CREATE_PROCESS[14] = "CreateProcess ";
static const char LOAD_DLL[8] = "LoadDll ";
static const char UNLOAD_DLL[10] = "UnloadDll ";
static const char CREATE_THREAD[13] = "CreateThread ";
static const char EXIT_THREAD[11] = "ExitThread ";
static const char EXIT_PROCESS[12] = "ExitProcess ";
static const char OUTPUT_DEBUG[18] = "OutputDebugString ";

// Newline & Applies non-bold/bright red to foreground
static const char CONSOLE_NRED_FORMAT[6] = "\n\x1b[31m";
static const char THREAD_TRIGGERD[23] = " triggered exception 0x";
static const char THREAD_NUMBER[8] = "Thread #";

static const wchar_t TMPENV[3] = L"TMP";
static const wchar_t GDB_EXE[] = L"gdb.exe";
static const wchar_t W64DBG[6] = L"w64dbg";
static const char GDB_DEFAULT[125] =
    "set bac l 100\n" // set backtrace limit 100
    "set con of\n" // set confirm off
    "set p th of\n" // set print thread-events off
    "set p i of\n" // set print inferior-events off
    "set p frame-i source-a\n" // set print frame-info source-and-location
    "set p en n\n" // set print entry-values no
    "set lo f NUL\n" // set logging file
    "set lo r\n" // set logging redirect
    "set lo d\n" // set logging debugredirect
    "set lo e\n" // set logging enabled
    "at "; // attach

// set pagination off & set width 0
static const char GDB_PRESERVE[20] = "\nset pa of\nset wi 0\n";
// set style enabled
static const char GDB_STYLE[11] = "\nset sty e\n";
// set print frame-arguments all & set print pretty
static const char GDB_FRAME_ARG[25] = "set p frame-a a\nset p pr\n";
// continue & set logging enabled off & backtrace
static const char GDB_CONTINUE[16] = "c\nset lo e of\nbt";

static const wchar_t GDB_BATCH[8] = L" --batch";
static const char GDB_QUIT[2] = "q\n";

// Applies non-bold/bright blue to foreground
static const char CONSOLE_BLUE_FORMAT[5] = "\x1b[34m";
// Applies non-bold/bright green to foreground
static const char CONSOLE_GREEN_FORMAT[5] = "\x1b[32m";

static const char EXCEPTION_IN[12] = "\x1b[m in \x1b[33m";
static const char EXCEPTION_AT[3] = "at ";
static const char EXCEPTION_FROM[5] = "from ";

static const char RIP[4] = "RIP ";
static const char _SLE_ERROR[88] =
"Invalid data was passed to the function that failed. This caused the application to fail";
static const char _SLE_MINORERROR[102] =
"Invalid data was passed to the function, but the error probably will not cause the application to fail";
static const char _SLE_WARNING[90] =
"Potentially invalid data was passed to the function, but the function completed processing";