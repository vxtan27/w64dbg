#pragma once

#include "bio.c"
#include "format.c"
#include "symbol.c"

#include <psapi.h>
#include <dbghelp.h>

#ifdef _MSC_VER
#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "dbghelp.lib")
#endif

#define PROCCREATIONFLAGS \
    CREATE_BREAKAWAY_FROM_JOB | \
    CREATE_DEFAULT_ERROR_MODE | \
    CREATE_PRESERVE_CODE_AUTHZ_LEVEL | \
    INHERIT_PARENT_AFFINITY

#define MAX_THREAD 64
#define MAX_DLL 16
#define MINGW 2
#define LATENCY 25

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
    SYMOPT_LOAD_LINES | \
    SYMOPT_NO_PUBLICS

#define W64DBG_HELP \
    "\n" \
    "W64DBG [ option... ] <executable> [ exec arg... ]\n" \
    "\n" \
    "Description:\n" \
    "    This tool is used to debug an executable on x64 Windows.\n" \
    "\n" \
    "Entries:\n" \
    "    option    one or more W64DBG options.\n" \
    "    exec arg  one or more executable arguments.\n" \
    "\n" \
    "Option List:\n" \
    "    /B        Ignore breakpoints.\n" \
    "    /D        Load the executable's PDB debug symbols.\n" \
    "    /G[-]     Load the executable's DWARF debug symbols.\n" \
    "    /O        Do not display OutputDebugString string.\n" \
    "    /S        Start the executable in a new console.\n" \
    "    /T        Wait for the specified period (in seconds).\n" \
    "    /V<n>     Display verbose debug information.\n" \

#define W64DBG_BAD_EXE_FORMAT " is not a valid Win32 application.\n"
#define W64DBG_FROM "from \x1b[32m"
#define W64DBG_AT "at \x1b[32m"