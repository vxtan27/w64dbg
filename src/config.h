/* Configuration */
#define DEFAULT_DEBUG_DWARF   FALSE
#define DEFAULT_BREAKPOINT    TRUE
#define DEFAULT_FIRSTBREAK    FALSE
#define DEFAULT_TIMEOUT       0
#define DEFAULT_VERBOSE       FALSE
#define DEFAULT_OUTPUT        TRUE
#define DEFAULT_START         FALSE
#define DEFAULT_HELP          FALSE
#define VALID_TIMEOUT         99999
#define INVALID_TIMEOUT       100000
#define MAX_THREAD            32
#define MAX_DLL               16
#define LATENCY               25

/* Buffer length */
#define PAGESIZE    4096
#define BUFLEN      8192
#define WBUFLEN     4096

#define CREATIONFLAGS \
    CREATE_UNICODE_ENVIRONMENT | \
    CREATE_BREAKAWAY_FROM_JOB | \
    CREATE_PRESERVE_CODE_AUTHZ_LEVEL | \
    CREATE_DEFAULT_ERROR_MODE

#define SYMOPTIONS \
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

/* MinGW options */
#define MINGW_KEEP 1
#define MINGW_NOKEEP 2

#define HELP \
    "Invalid syntax.\n" \
    "Usage: W64DBG [options] <executable> [exec-args]\n" \
    "\n" \
    "Description:\n" \
    "    A native debugging utility for x64 Windows.\n" \
    "\n" \
    "Entries:\n" \
    "    options       Options control behavior.\n" \
    "    executable    Target executable file.\n" \
    "    exec-args     Target executable arguments.\n" \
    "\n" \
    "Options:\n" \
    "    /B            Ignore breakpoints.\n" \
    "    /D            Load PDB debug symbols.\n" \
    "    /G[+]         Load DWARF debug symbols.\n" \
    "    /O            Suppress OutputDebugString.\n" \
    "    /S            Open in a new console window.\n" \
    "    /T<n>         Wait for input (seconds).\n" \
    "    /V{0|1|2}     Set output verbosity.\n"

#define VALUE_EXPECTED "Value expected for '"
#define TIMEOUT_INVALID "Invalid value for timeout (  ) specified. Valid range is -1 to 99999.\n"
#define _INVALID_ARGUMENT "Invalid argument/option - '"
#define INVALID_ARGUMENT_ "'.\n"

#define PATHENV L"PATH"
#define EXTENSION L".exe"

#define CREATE_PROCESS "CreateProcess "
#define LOAD_DLL "LoadDll "
#define UNLOAD_DLL "UnloadDll "
#define CREATE_THREAD "CreateThread "
#define EXIT_THREAD "ExitThread "
#define EXIT_PROCESS "ExitProcess "
#define OUTPUT_DEBUG "OutputDebugString "

// Newline & Applies non-bold/bright red to foreground
#define CONSOLE_NRED_FORMAT "\n\x1b[31m"
#define THREAD_TRIGGERD " triggered exception 0x"
#define THREAD_NUMBER "Thread #"

// #define OBJECT_MANAGER_NAMESPACE L"\\??\\"
// #define OBJECT_MANAGER_NAMESPACE "\\\0?\0?\0\\"

#define OBJECT_MANAGER_NAMESPACE GDB_COMMAND_LINE + 14
#define OBJECT_MANAGER_NAMESPACE_LEN 8
#define OBJECT_MANAGER_NAMESPACE_WLEN (OBJECT_MANAGER_NAMESPACE_LEN >> 1)

#define TMPENV L"TMP"
#define GDB_EXE L"gdb.exe"
#define W64DBG L"w64dbg"

// GDB command line arguments
#define GDB_COMMAND_LINE L"gdb.exe -q \??\\"
// Returns all attributes to the default state prior to modification
#define CONSOLE_DEFAULT_FORMAT "\x1b[m"

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

// set pagination off & set width 0
#define GDB_PRESERVE "\nset pa of\nset wi 0\n"
// set style enabled
#define GDB_STYLE "\nset sty e\n"
// set print frame-arguments all & set print frame-info source-and-location & set print pretty
#define GDB_FRAME_ARG "set p frame-a a\nset p frame-i source-a\nset p pr\n"
// continue & set logging enabled off & backtrace
#define GDB_CONTINUE "c\nset lo e of\nbt"

#define GDB_BATCH L" --batch"
#define GDB_QUIT "q\n"

// Applies non-bold/bright blue to foreground
#define CONSOLE_BLUE_FORMAT "\x1b[34m"
// Applies non-bold/bright green to foreground
#define CONSOLE_GREEN_FORMAT "\x1b[32m"

#define EXCEPTION_IN "\x1b[m in \x1b[33m"
#define EXCEPTION_AT "at "
#define EXCEPTION_FROM "from "

#define RIP "RIP "
#define _SLE_ERROR "Invalid data was passed to the function that failed. This caused the application to fail"
#define _SLE_MINORERROR "Invalid data was passed to the function, but the error probably will not cause the application to fail"
#define _SLE_WARNING "Potentially invalid data was passed to the function, but the function completed processing"

/*
    Faster Builds
*/

#define WIN32_LEAN_AND_MEAN

#define NOGDICAPMASKS
// #define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NOCRYPT
#define NOIME
#define NOFONTSIG

#define NOMMIDS
#define NONEWWAVE
#define NONEWRIFF
#define NOJPEGDIB
#define NONEWIC
#define NOBITMAP

/* #define NOUSER */
#define NOTOOLBAR
#define NOREBAR
#define NOUPDOWN
#define NOTOOLTIPS
#define NOSTATUSBAR
#define NOMENUHELP
#define NOTRACKBAR
#define NODRAGLIST
#define NOPROGRESS
#define NOHOTKEY
#define NOHEADER
#define NOIMAGEAPIS
#define NOLISTVIEW
#define NOTREEVIEW
#define NOUSEREXCONTROLS
#define NOTABCONTROL
#define NOANIMATE
#define NOMONTHCAL
#define NODATETIMEPICK
#define NOIPADDRESS
#define NOPAGESCROLLER
#define NONATIVEFONTCTL
#define NOBUTTON
#define NOSTATIC
#define NOEDIT
#define NOLISTBOX
#define NOCOMBOBOX
#define NOSCROLLBAR
#define NOTASKDIALOG
#define NOMUI
#define NOTRACKMOUSEEVENT
// #define NORESOURCE
#define NODESKTOP
#define NOWINDOWSTATION
#define NOSECURITY
#define NONCMESSAGES
#define NOMDI
#define NOSYSPARAMSINFO
#define NOWINABLE
#define NO_STATE_FLAGS

#define NODDRAWGDI
#define NODDEMLSPY
#define NO_COMMCTRL_DA
#define NO_DSHOW_STRSAFE
#define NODXINCLUDES
#define NODXMALLOC
#define NODXLCL
#define NODX95TYPES
#define NO_BASEINTERFACE_FUNCS
#define NOWINBASEINTERLOCK
#define NO_MEDIA_ENGINE_FACTORY
#define NO_INTSHCUT_GUIDS
#define NO_SHDOCVW_GUIDS
#define NO_WRAPPERS_FOR_ILCREATEFROMPATH
#define NO_SHOBJIDL_SORTDIRECTION
#define NO_WCN_PKEYS
#define NOEXTAPI
#define NO_WIA_DEBUG
// #define NOAPISET

/* NOSHLWAPI */
#define NO_SHLWAPI_STRFCNS
#define NO_SHLWAPI_PATH
#define NO_SHLWAPI_REG
#define NO_SHLWAPI_STREAM
#define NO_SHLWAPI_HTTP
#define NO_SHLWAPI_ISOS
#define NO_SHLWAPI_GDI

#define NOCOMPMAN
#define NODRAWDIB
#define NOVIDEO
#define NOAVIFMT
#define NOMMREG
#define NOAVIFILE
#define NOMCIWND
#define NOAVICAP
#define NOMSACM