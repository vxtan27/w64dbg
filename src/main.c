/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

#include "copyright.c"
#include "license.c"
#include "winput.c" // Waiting for input
#include "strfmt.c" // String formatting
#include "symen.c" // Symbols enumeration
#include <psapi.h>

#define MINGW 2
#define W64DBG_DEFAULT_TIMEOUT 0
#define W64DBG_DEFAULT_DEBUG FALSE
#define W64DBG_DEFAULT_BREAKPOINT TRUE
#define W64DBG_DEFAULT_FIRSTBREAK FALSE
#define W64DBG_DEFAULT_OUTPUT TRUE
#define W64DBG_DEFAULT_VERBOSE FALSE
#define W64DBG_DEFAULT_START FALSE
#define W64DBG_DEFAULT_HELP FALSE

#define W64DBG_USAGE \
    "Usage: W64DBG [options] <executable> [exec-args]\n"

#define W64DBG_INVALID_USAGE \
    "ERROR: Invalid syntax.\n" \
    W64DBG_USAGE

#define W64DBG_HELP \
    "\n" \
    W64DBG_USAGE \
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
    "    /T            Wait for input (seconds).\n" \
    "    /V<n>         Set output verbosity.\n"

#define W64DBG_VALUE_EXPECTED "ERROR: Value expected for '"
#define W64DBG_ERROR_INVALID_TIMEOUT "ERROR: Invalid value for timeout (  ) specified. Valid range is -1 to 99999.\n"
#define W64DBG_BAD_EXE_FORMAT " is not a valid Win32 application.\n"
#define W64DBG_ERROR_INVALID W64DBG_ERROR_INVALID_TIMEOUT

#define MAX_THREAD 32
#define MAX_DLL 16

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
    SYMOPT_LOAD_LINES | \
    SYMOPT_NO_PUBLICS

#define LATENCY 25
#define W64DBG_DEFAULT_LEN 125
#define W64DBG_DEFAULT_OFFSET W64DBG_DEFAULT_LEN

// https://hero.handmade.network/forums/code-discussion/t/94-guide_-_how_to_avoid_c_c++_runtime_on_windows

__declspec(noreturn)
void __stdcall main(void)
{
    PWSTR pNext;
    size_t temp, len;
    char buffer[BUFLEN];
    ULONG UTF8StringActualByteCount;

    char *p = buffer;
    int timeout = W64DBG_DEFAULT_TIMEOUT;
    PWSTR pCmdLine = wcschr(GetCommandLineW(), ' ');
    char breakpoint = W64DBG_DEFAULT_BREAKPOINT,
    firstbreak = W64DBG_DEFAULT_FIRSTBREAK,
    verbose = W64DBG_DEFAULT_VERBOSE,
    output = W64DBG_DEFAULT_OUTPUT,
    debug = W64DBG_DEFAULT_DEBUG,
    start = W64DBG_DEFAULT_START,
    help = W64DBG_DEFAULT_HELP;

    if (pCmdLine)
    {
        pNext = pCmdLine;
        len = wcslen(pCmdLine);
        // Modified for processing command-line arguments
        *(pCmdLine + len) = ' ';

        while (TRUE)
        {
            while (*pNext == ' ') ++pNext; // Skip spaces

            if ((*pNext != '/' && *pNext != '-') ||
                pCmdLine + len < pNext) break;

            switch (*(pNext + 1))
            {
                case 'B':
                case 'b':
                    if (*(pNext + 2) == ' ')
                    {
                        breakpoint = FALSE;
                        pNext += 3;
                        continue;
                    }

                case 'D':
                case 'd':
                    if (*(pNext + 2) == ' ')
                    {
                        debug = TRUE;
                        pNext += 3;
                        continue;
                    }

                case 'G':
                case 'g':
                    if (*(pNext + 2) == ' ')
                    {
                        debug = MINGW;
                        pNext += 3;
                        continue;
                    } else if (*(pNext + 3) == '-' && *(pNext + 3) == ' ')
                    {
                        debug = MINGW + 1;
                        pNext += 4;
                        continue;
                    }

                case 'O':
                case 'o':
                    if (*(pNext + 2) == ' ')
                    {
                        output = FALSE;
                        pNext += 3;
                        continue;
                    }

                case 'S':
                case 's':
                    if (*(pNext + 2) == ' ')
                    {
                        start = TRUE;
                        pNext += 3;
                        continue;
                    }

                case 'T':
                case 't':
                    wchar_t *cmd = pNext;
                    pNext += 2;

                    while (*pNext == ' ') ++pNext; // Skip spaces

                    temp = pCmdLine + len + 1 - pNext;

                    if (temp <= 0)
                    {
                        memcpy(p, W64DBG_VALUE_EXPECTED,
                            strlen(W64DBG_VALUE_EXPECTED));
                        p += strlen(W64DBG_VALUE_EXPECTED);
                        *p++ = *cmd;
                        *p++ = *(cmd + 1);
                        *p++ = '\'';
                        *p++ = '\n';
                    } else
                    {
                        if ((timeout = __builtin_wcstol(pNext)) > 99999)
                        {
                            memcpy(p, W64DBG_ERROR_INVALID_TIMEOUT, strlen(W64DBG_ERROR_INVALID_TIMEOUT));
                            p += strlen(W64DBG_ERROR_INVALID_TIMEOUT);
                            *(p - 43) = *cmd;
                            *(p - 42) = *(cmd + 1);
                        }

                        pNext = (wchar_t *) __builtin_wmemchr(pNext, ' ', temp) + 1;
                    }

                    continue;

                case 'V':
                case 'v':
                    if (*(pNext + 2) == ' ')
                    {
                        verbose = 3;
                        pNext += 3;
                    } else if (iswdigit(*(pNext + 2)) && *(pNext + 3) == ' ')
                    {
                        verbose = *(pNext + 2) - '0';
                        pNext += 4;
                    } else break;

                    continue;

                case '?':
                    if (*(pNext + 2) == ' ')
                    {
                        help = TRUE;
                        pNext += 3;
                        continue;
                    }
            }

            memcpy(p, "ERROR: Invalid argument/option - '", 34);
            p += 34;

            temp = __builtin_wmemchr(pNext, ' ',
                pCmdLine + len + 1 - pNext) - pNext;

            RtlUnicodeToUTF8N(p, buffer + BUFLEN - p,
                &UTF8StringActualByteCount, pNext, temp << 1);
            p += UTF8StringActualByteCount;
            pNext += temp + 1;

            memcpy(p, "'.\n", 3);
            p += 3;
        }
    }

    if (help)
    {
        memcpy(p, W64DBG_HELP, strlen(W64DBG_HELP));
        p += strlen(W64DBG_HELP);
    } else if (!pCmdLine || pCmdLine + len < pNext)
    { // No executable specified
        memcpy(p, W64DBG_INVALID_USAGE,
            strlen(W64DBG_INVALID_USAGE));
        p += strlen(W64DBG_INVALID_USAGE);
    }

    char Console;
    HANDLE hStdout;
    IO_STATUS_BLOCK IoStatusBlock;

    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    Console = GetFileType(hStdout) == FILE_TYPE_CHAR;

    if (Console) SetConsoleOutputCP(65001);

    if (p != buffer)
    {
        NtWriteFile(hStdout, NULL, NULL, NULL,
            &IoStatusBlock, buffer, p - buffer, NULL, NULL);
        ExitProcess(1);
    }

    DWORD cDirLen;
    ULONG PathLen;
    wchar_t PATH[WBUFLEN];
    wchar_t ApplicationName[WBUFLEN];
    UNICODE_STRING Variable, Value;

    Variable.Length = 8;
    Variable.Buffer = L"PATH";
    cDirLen = RtlGetCurrentDirectory_U(sizeof(PATH), PATH);
    Value.MaximumLength = sizeof(PATH) - cDirLen - 2;
    cDirLen >>= 1;
    PATH[cDirLen] = ';';
    Value.Buffer = PATH + (cDirLen) + 1;
    RtlQueryEnvironmentVariable_U(NULL, &Variable, &Value);

    temp = __builtin_wmemchr(pNext, ' ',
        pCmdLine + len + 1 - pNext) - pNext;
    *(pNext + temp) = '\0';

    *(pCmdLine + len) = '\0';

    // Check if executable exists
    if (!(PathLen = RtlDosSearchPath_U(PATH, pNext, L".exe",
        sizeof(ApplicationName), ApplicationName, NULL)))
    {
        NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock,
            W64DBG_FILE_NOT_FOUND, strlen(W64DBG_FILE_NOT_FOUND), NULL, NULL);
        ExitProcess(1);
    }

    DWORD type;

    if (!GetBinaryTypeW(ApplicationName, &type) ||
        (type != SCS_32BIT_BINARY && type != SCS_64BIT_BINARY))
    { // Check if executable format (x86-64)
        memcpy(buffer, W64DBG_ERROR_INVALID, 7);

        ULONG UTF8StringActualByteCount;
        RtlUnicodeToUTF8N(buffer + 7, BUFLEN - 7,
            &UTF8StringActualByteCount, ApplicationName, PathLen);

        memcpy(buffer + 7 + UTF8StringActualByteCount,
            W64DBG_BAD_EXE_FORMAT, strlen(W64DBG_BAD_EXE_FORMAT));
        NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock, buffer,
            7 + strlen(W64DBG_BAD_EXE_FORMAT) + UTF8StringActualByteCount, NULL, NULL);
        ExitProcess(1);
    }

    HANDLE hProcess;
    HANDLE hFile[MAX_DLL];
    DEBUG_EVENT DebugEvent;
    HANDLE hThread[MAX_THREAD];
    PROCESS_INFORMATION processInfo;
    LPVOID BaseOfDll[MAX_DLL] = {};
    STARTUPINFOW startupInfo = {sizeof(startupInfo)};

    *(pNext + temp) = ' ';

    if (debug < MINGW)
    {
        // The system closes the handles to the process and thread
        // when the debugger calls the ContinueDebugEvent function
        CreateProcessW(ApplicationName, pNext, NULL, NULL, FALSE,
            start ? CreationFlags | CREATE_NEW_CONSOLE |
            DEBUG_ONLY_THIS_PROCESS : CreationFlags | DEBUG_ONLY_THIS_PROCESS,
            NULL, NULL, &startupInfo, &processInfo);

        NtClose(processInfo.hThread);
        NtClose(processInfo.hProcess);
        WaitForDebugEvent(&DebugEvent, INFINITE);
        if (debug == TRUE)
        {
            hFile[0] = DebugEvent.u.CreateProcessInfo.hFile;
            BaseOfDll[0] = DebugEvent.u.CreateProcessInfo.lpBaseOfImage;
        } else NtClose(DebugEvent.u.CreateProcessInfo.hFile);
        hProcess = DebugEvent.u.CreateProcessInfo.hProcess;
        hThread[0] = DebugEvent.u.CreateProcessInfo.hThread;
    } else
    {
        // The system closes the handles to the process and thread
        // when the debugger calls the DebugActiveProcessStop function
        CreateProcessW(ApplicationName, pNext, NULL, NULL, FALSE,
            start ? CreationFlags | CREATE_NEW_CONSOLE : CreationFlags,
            NULL, NULL, &startupInfo, &processInfo);

        DebugActiveProcess(processInfo.dwProcessId);
        WaitForDebugEvent(&DebugEvent, INFINITE);
        NtClose(DebugEvent.u.CreateProcessInfo.hFile);
        hThread[0] = processInfo.hThread;
        hProcess = processInfo.hProcess;
    }

    if (verbose >= 2)
    {
        memcpy(buffer, "CreateProcess ", 14);
        p = debug_ultoa(DebugEvent.dwProcessId, buffer + 14);
        *p = 'x';
        p = debug_ultoa(DebugEvent.dwThreadId, p + 1);
        *p = '\n';
        NtWriteFile(hStdout, NULL, NULL, NULL,
            &IoStatusBlock, buffer, p - buffer + 1, NULL, NULL);
    }

    // DBG_EXCEPTION_NOT_HANDLED
    // https://learn.microsoft.com/en-us/windows/win32/api/debugapi/nf-debugapi-continuedebugevent
    ContinueDebugEvent(DebugEvent.dwProcessId, DebugEvent.dwThreadId, DBG_CONTINUE);

    char bWow64;
    unsigned char i;
    ULONG_PTR wow64;
    DWORD dwThreadId[MAX_THREAD] = {};

    NtQueryInformationProcess(hProcess, ProcessWow64Information,
        &wow64, sizeof(ULONG_PTR), NULL);

    // x86 process trigger more than one breakpoints
    if ((bWow64 = !!wow64) && debug < MINGW) --firstbreak;

    dwThreadId[0] = DebugEvent.dwThreadId;

    while (TRUE)
    {
        WaitForDebugEvent(&DebugEvent, INFINITE);

        // https://learn.microsoft.com/en-us/windows/win32/debug/debugging-events
        switch (DebugEvent.dwDebugEventCode)
        {

            case LOAD_DLL_DEBUG_EVENT:
                if (verbose >= 2)
                {
                    DWORD Len;
                    wchar_t Tmp[WBUFLEN];

                    memcpy(buffer, "LoadDll ", 8);
                    Len = GetFinalPathNameByHandleW(DebugEvent.u.LoadDll.hFile,
                        Tmp, WBUFLEN, FILE_NAME_OPENED);

                    ULONG UTF8StringActualByteCount;
                    RtlUnicodeToUTF8N(buffer + 8, BUFLEN - 8,
                        &UTF8StringActualByteCount, Tmp, Len << 1);

                    buffer[UTF8StringActualByteCount + 8] = '\n';
                    NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock,
                        buffer, UTF8StringActualByteCount + 9, NULL, NULL);
                }

                // Find storage position
                for (i = 0; i < MAX_DLL; ++i) if (!BaseOfDll[i])
                {
                    hFile[i] = DebugEvent.u.LoadDll.hFile;
                    BaseOfDll[i] = DebugEvent.u.LoadDll.lpBaseOfDll;
                    break;
                }

                break;

            case UNLOAD_DLL_DEBUG_EVENT:
                // Find specific DLL
                for (i = 0; i < MAX_DLL; ++i)
                    if (DebugEvent.u.UnloadDll.lpBaseOfDll == BaseOfDll[i])
                {
                    if (verbose >= 2)
                    {
                        DWORD Len;
                        wchar_t Tmp[WBUFLEN];

                        memcpy(buffer, "UnloadDll ", 10);
                        Len = GetFinalPathNameByHandleW(DebugEvent.u.LoadDll.hFile,
                            Tmp, WBUFLEN, FILE_NAME_OPENED);

                        ULONG UTF8StringActualByteCount;
                        RtlUnicodeToUTF8N(buffer + 10, BUFLEN - 10,
                            &UTF8StringActualByteCount, Tmp, Len << 1);

                        buffer[UTF8StringActualByteCount + 10] = '\n';
                        NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock,
                            buffer, UTF8StringActualByteCount + 11, NULL, NULL);
                    }

                    NtClose(hFile[i]);
                    BaseOfDll[i] = 0;
                    break;
                }

                break;

            case CREATE_THREAD_DEBUG_EVENT:
                if (verbose >= 2)
                {
                    memcpy(buffer, "CreateThread ", 13);
                    p = debug_ultoa(DebugEvent.dwProcessId, buffer + 13);
                    *p = 'x';
                    p = debug_ultoa(DebugEvent.dwThreadId, p + 1);
                    *p = '\n';
                    NtWriteFile(hStdout, NULL, NULL, NULL,
                        &IoStatusBlock, buffer, p - buffer + 1, NULL, NULL);
                }

                // Find storage position
                for (i = 0; i < MAX_THREAD; ++i) if (!dwThreadId[i])
                {
                    hThread[i] = DebugEvent.u.CreateThread.hThread;
                    dwThreadId[i] = DebugEvent.dwThreadId;
                    break;
                }

                break;

            case EXIT_THREAD_DEBUG_EVENT:
                if (verbose >= 2)
                {
                    memcpy(buffer, "ExitThread ", 11);
                    p = debug_ultoa(DebugEvent.dwProcessId, buffer + 11);
                    *p = 'x';
                    p = debug_ultoa(DebugEvent.dwThreadId, p + 1);
                    *p = '\n';
                    NtWriteFile(hStdout, NULL, NULL, NULL,
                        &IoStatusBlock, buffer, p - buffer + 1, NULL, NULL);
                }

                // Find specific thread
                for (i = 0; i < MAX_THREAD; ++i) if (DebugEvent.dwThreadId == dwThreadId[i])
                {
                    dwThreadId[i] = 0;
                    break;
                }

                break;

            case EXIT_PROCESS_DEBUG_EVENT:
                if (verbose >= 2)
                {
                    memcpy(buffer, "ExitProcess ", 12);
                    p = debug_ultoa(DebugEvent.dwProcessId, buffer + 12);
                    *p = 'x';
                    p = debug_ultoa(DebugEvent.dwThreadId, p + 1);
                    *p = '\n';
                    NtWriteFile(hStdout, NULL, NULL, NULL,
                        &IoStatusBlock, buffer, p - buffer + 1, NULL, NULL);
                }

                // Cleanup
                if (debug >= MINGW)
                {
                    NtClose(hProcess);
                    NtClose(hThread[0]);
                }

                for (i = 0; i < MAX_DLL; ++i)
                    if (BaseOfDll[i]) NtClose(hFile[i]);

                if (timeout)
                {
                    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
                    WaitForInputOrTimeout(hStdin, hStdout,
                        GetFileType(hStdin) == FILE_TYPE_CHAR, timeout);
                }

                NtTerminateProcess(hProcess, DebugEvent.u.Exception.ExceptionRecord.ExceptionCode);
                ContinueDebugEvent(DebugEvent.dwProcessId, DebugEvent.dwThreadId, DBG_CONTINUE);
                ExitProcess(0);

            case OUTPUT_DEBUG_STRING_EVENT:
                if (verbose >= 2)
                {
                    memcpy(buffer, "OutputDebugString ", 18);
                    p = debug_ultoa(DebugEvent.dwProcessId, buffer + 18);
                    *p = 'x';
                    p = debug_ultoa(DebugEvent.dwThreadId, p + 1);
                    *p = '\n';
                    NtWriteFile(hStdout, NULL, NULL, NULL,
                        &IoStatusBlock, buffer, p - buffer + 1, NULL, NULL);
                }

                if (output == TRUE)
                {
                    NtReadVirtualMemory(hProcess,
                        DebugEvent.u.DebugString.lpDebugStringData,
                        buffer, --DebugEvent.u.DebugString.nDebugStringLength, NULL);
                    NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock, buffer,
                        DebugEvent.u.DebugString.nDebugStringLength, NULL, NULL);
                }

                break;

            case EXCEPTION_DEBUG_EVENT:
                // Skip first-chance breakpoints
                if ((DebugEvent.u.Exception.ExceptionRecord.ExceptionCode == 0x80000003 || // EXCEPTION_BREAKPOINT
                    DebugEvent.u.Exception.ExceptionRecord.ExceptionCode == 0x4000001F ) && // STATUS_WX86_BREAKPOINT
                    ((breakpoint == FALSE) || (breakpoint == TRUE && ++firstbreak <= 1)))
                    break;

                // Check if not first-chance
                if (!DebugEvent.u.Exception.dwFirstChance)
                {
                    ContinueDebugEvent(DebugEvent.dwProcessId, DebugEvent.dwThreadId, 0x80010001L);
                    continue;
                }

                // Find thread where exception occured
                for (i = 0; i < MAX_THREAD; ++i) if (DebugEvent.dwThreadId == dwThreadId[i])
                    break;

                memcpy(buffer, "Thread #", 8);
                if (DebugEvent.dwThreadId == dwThreadId[i])
                {
                    buffer[8] = '0' + (i + 1) / 10;
                    buffer[9] = '0' + (i + 1) % 10;
                    p = buffer + 10;
                } else p = debug_ultoa(DebugEvent.dwThreadId, buffer + 7);
                memcpy(p, " caused ", 8);
                p = FormatDebugException(&DebugEvent.u.Exception.ExceptionRecord, p + 8, bWow64);
                *p++ = '\n';

                if (Console)
                {
                    memcpy(p, "\x1b[31m", 5);
                    p += 5;
                }

                char *pre = p;

                p = FormatVerboseDebugException(p,
                    DebugEvent.u.Exception.ExceptionRecord.ExceptionCode);

                if (Console)
                {
                    memcpy(p, "\x1b[m", 3);
                    p += 3;
                    SetConsoleMode(hStdout, ENABLE_PROCESSED_OUTPUT |
                        ENABLE_WRAP_AT_EOL_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
                }

                    if (p != pre) *p++ = '\n';

                if (!(DebugEvent.u.Exception.ExceptionRecord.ExceptionCode & EXCEPTION_NONCONTINUABLE) &&
                    (DebugEvent.u.Exception.ExceptionRecord.ExceptionCode == 0x406D1388 ||
                    DebugEvent.u.Exception.ExceptionRecord.ExceptionCode == 0xE06D7363 || // STATUS_CPP_EH_EXCEPTION
                    DebugEvent.u.Exception.ExceptionRecord.ExceptionCode == 0xE0434f4D)) // STATUS_CLR_EXCEPTION
                    ContinueDebugEvent(DebugEvent.dwProcessId, DebugEvent.dwThreadId, DBG_CONTINUE);
                else if (debug < MINGW)
                    ContinueDebugEvent(DebugEvent.dwProcessId, DebugEvent.dwThreadId, 0x80010001L);
                else
                {
                    ULONG PathLen;
                    wchar_t ApplicationName[WBUFLEN];

                    // Check if executable exists
                    if ((PathLen = RtlDosSearchPath_U(PATH, L"gdb.exe", NULL,
                        sizeof(ApplicationName), ApplicationName, NULL)))
                    {
                        NtWriteFile(hStdout, NULL, NULL, NULL,
                            &IoStatusBlock, buffer, p - buffer, NULL, NULL);
                        NtSuspendProcess(hProcess);
                        DebugActiveProcessStop(DebugEvent.dwProcessId);

                        HANDLE hTemp;
                        UNICODE_STRING String;
                        LARGE_INTEGER ByteOffset;
                        OBJECT_ATTRIBUTES ObjectAttributes;
                        wchar_t CommandLine[MAX_PATH + 18] = L"gdb.exe -q -x=\\??\\";
                        UNICODE_STRING Variable, Value;

                        Variable.Length = 6;
                        Variable.Buffer = L"TMP";
                        Value.MaximumLength = sizeof(CommandLine) - 18;
                        Value.Buffer = &CommandLine[18];
                        RtlQueryEnvironmentVariable_U(NULL, &Variable, &Value);
                        Value.Length >>= 1;

                        // Ensure correct DOS path
                        if (CommandLine[17 + Value.Length] != '\\')
                        {
                            CommandLine[18 + Value.Length] = '\\';
                            ++Value.Length;
                        }

                        memcpy(&CommandLine[18 + Value.Length], L"w64dbg", 12);
                        String.Length = (4 + Value.Length + 6) << 1;
                        String.Buffer = &CommandLine[14];
                        InitializeObjectAttributes(&ObjectAttributes,
                            &String, OBJ_CASE_INSENSITIVE, NULL, NULL);
                        NtCreateFile(&hTemp, GENERIC_WRITE | SYNCHRONIZE, &ObjectAttributes,
                            &IoStatusBlock, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN_IF,
                            FILE_WRITE_THROUGH | FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT,
                            NULL, 0);

                        // First time run
                        if (IoStatusBlock.Information == FILE_CREATED)
                            NtWriteFile(hTemp, NULL, NULL, NULL,
                                &IoStatusBlock,
                                "set bac l 100\n" // set backtrace limit 100
                                "set p th of\n" // set print thread-events off
                                "set p i of\n" // set print inferior-events off
                                "set lo r\n" // set logging redirect
                                "set lo d\n" // set logging debugredirect
                                "set lo f NUL\n" // set logging file
                                "set lo e\n" // set logging enabled
                                "set con of\n" // set confirm off
                                "set p en n\n" // set print entry-values no
                                "set p frame-i source-a\n" // set print frame-info source-and-location
                                "at " // attach
                                , W64DBG_DEFAULT_LEN, NULL, NULL);

                        space_ultoa(DebugEvent.dwProcessId, buffer);
                        p = buffer + 5;
                        *p++ = '\n';

                        if (debug == MINGW + 1)
                        {
                            memcpy(p,
                                "set pa of\n" // set pagination off
                                "set wi 0" // set width 0
                                , 18);
                            p += 18;
                        } else
                        {
                            // set style enabled
                            memcpy(p, "set sty e", 9);
                            p += 9;
                        }

                        *p++ = '\n';

                        if (verbose >= 1)
                        {
                            // set print frame-arguments all
                            memcpy(p,
                                "set p frame-a a\n"
                                "set p pr\n"
                                , 25);
                            p += 25;
                        }

                        // continue & set logging enabled off & backtrace
                        memcpy(p, "c\nset lo e of\nbt", 16);
                        p += 16;

                        if (verbose >= 1)
                        {
                            *p++ = ' ';
                            *p++ = 'f';
                        }

                        ByteOffset.QuadPart = W64DBG_DEFAULT_OFFSET;
                        NtWriteFile(hTemp, NULL, NULL, NULL, &IoStatusBlock,
                            buffer, p - buffer, &ByteOffset, NULL);
                        ByteOffset.QuadPart +=  p - buffer;
                        NtSetInformationFile(hTemp, &IoStatusBlock, &ByteOffset,
                            sizeof(LARGE_INTEGER), 20); // FileEndOfFileInformation
                        NtClose(hTemp);

                        if (debug == MINGW && !timeout)
                            memcpy(&CommandLine[18 + Value.Length + 6], L" --batch", 16);

                        DWORD_PTR ProcDbgPt;
                        LARGE_INTEGER DelayInterval;

                        CreateProcessW(ApplicationName, CommandLine, NULL, NULL, FALSE,
                            CreationFlags, NULL, NULL, &startupInfo, &processInfo);
                        NtClose(processInfo.hThread);
                        NtClose(hThread[0]);
                        // Convert seconds to 100-nanosecond units
                        // (negative for relative time)
                        DelayInterval.QuadPart = -(LATENCY * 10000);

                        while (TRUE)
                        { // Wait until GDB attachs
                            NtDelayExecution(FALSE, &DelayInterval);
                            NtQueryInformationProcess(hProcess, ProcessDebugPort,
                                &ProcDbgPt, sizeof(ProcDbgPt), NULL);
                            if (ProcDbgPt) break;
                        }

                        HANDLE hStdin;
                        char StdinConsole;

                        NtResumeProcess(hProcess);
                        // marks deletion on close, prevent writting to disk
                        NtClose(hProcess);

                        if (timeout)
                        { // Quit GDB first
                            hStdin = GetStdHandle(STD_INPUT_HANDLE);
                            StdinConsole = GetFileType(hStdin) == FILE_TYPE_CHAR;

                            if (StdinConsole)
                            {
                                DWORD dwWriten;
                                INPUT_RECORD InputRecord[2];

                                InputRecord[0].EventType = KEY_EVENT;
                                InputRecord[0].Event.KeyEvent.bKeyDown = TRUE;
                                InputRecord[0].Event.KeyEvent.wRepeatCount = 1;
                                InputRecord[0].Event.KeyEvent.uChar.AsciiChar = 'q';

                                InputRecord[1].EventType = KEY_EVENT;
                                InputRecord[1].Event.KeyEvent.bKeyDown = TRUE;
                                InputRecord[1].Event.KeyEvent.wRepeatCount = 1;
                                InputRecord[1].Event.KeyEvent.uChar.AsciiChar = '\n';

                                WriteConsoleInputW(hStdin,
                                    InputRecord, 2, &dwWriten);
                            } else NtWriteFile(hStdout, NULL, NULL, NULL,
                                &IoStatusBlock, "q\n", 2, NULL, NULL);
                        }

                        NtWaitForSingleObject(processInfo.hProcess, FALSE, NULL);
                        NtClose(processInfo.hProcess);

                        if (timeout) WaitForInputOrTimeout(hStdin,
                            hStdout, StdinConsole, timeout);

                        if (verbose >= 2)
                        {
                            memcpy(buffer, "ExitProcess ", 12);
                            p = debug_ultoa(DebugEvent.dwProcessId, buffer + 12);
                            *p = 'x';
                            p = debug_ultoa(DebugEvent.dwThreadId, p + 1);
                            *p = '\n';
                            NtWriteFile(hStdout, NULL, NULL, NULL,
                                &IoStatusBlock, buffer, p - buffer + 1, NULL, NULL);
                        }

                        ExitProcess(0);
                    } else if (verbose >= 3)
                    {
                        memcpy(p, W64DBG_FILE_NOT_FOUND,
                            strlen(W64DBG_FILE_NOT_FOUND));
                        p += strlen(W64DBG_FILE_NOT_FOUND);
                    }
                }

                if (DebugEvent.dwThreadId != dwThreadId[i])
                {
                    NtWriteFile(hStdout, NULL, NULL, NULL,
                        &IoStatusBlock, buffer, p - buffer, NULL, NULL);
                    continue;
                }

                DWORD MachineType;
                STACKFRAME64 StackFrame;
                FILE_STANDARD_INFORMATION FileInfo;

                SymSetOptions(debug == TRUE ? _DebugSymOptions : NDebugSymOptions);
                SymInitialize(hProcess, NULL, FALSE);

                for (unsigned char j = 0; j < MAX_DLL; ++j) if (BaseOfDll[j])
                {
                    NtQueryInformationFile(hFile[j], &IoStatusBlock,
                        &FileInfo, sizeof(FileInfo), 5); // FileStandardInformation
                    SymLoadModuleEx(hProcess,
                        hFile[j],
                        NULL,
                        NULL,
                        (DWORD64) BaseOfDll[j],
                        FileInfo.EndOfFile.LowPart,
                        NULL,
                        0);
                }

                CONTEXT Context;

                if (bWow64)
                {
                    ((PWOW64_CONTEXT) &Context)->ContextFlags = WOW64_CONTEXT_ALL;
                    Wow64GetThreadContext(hThread[i], (PWOW64_CONTEXT) &Context);
                    // Find no replacable NTAPI
                    MachineType = IMAGE_FILE_MACHINE_I386;
                    StackFrame.AddrPC.Offset = ((PWOW64_CONTEXT) &Context)->Eip;
                    StackFrame.AddrStack.Offset = ((PWOW64_CONTEXT) &Context)->Esp;
                    StackFrame.AddrFrame.Offset = ((PWOW64_CONTEXT) &Context)->Ebp;
                } else
                {
                    Context.ContextFlags = CONTEXT_ALL;
                    NtGetContextThread(hThread[i], &Context);
                    MachineType = IMAGE_FILE_MACHINE_AMD64;
                    StackFrame.AddrPC.Offset = Context.Rip;
                    StackFrame.AddrStack.Offset = Context.Rsp;
                    StackFrame.AddrFrame.Offset = Context.Rbp;
                }

                char Success;
                DWORD Displacement;
                char Symbol[BUFLEN];
                unsigned char count;
                PSYMBOL_INFOW pSymbol;
                IMAGEHLP_LINEW64 Line;
                DWORD64 Displacement64;
                wchar_t Tmp[WBUFLEN];
                USERCONTEXT UserContext;
                ULONG UTF8StringActualByteCount;

                count = 0;
                pSymbol = (PSYMBOL_INFOW) Symbol;
                Line.SizeOfStruct = sizeof(Line);
                StackFrame.AddrPC.Mode = AddrModeFlat;
                StackFrame.AddrStack.Mode = AddrModeFlat;
                StackFrame.AddrFrame.Mode = AddrModeFlat;
                pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
                pSymbol->MaxNameLen = (BUFLEN - sizeof(SYMBOL_INFO)) >> 1;

                if (debug == TRUE)
                {
                    UserContext.hProcess = hProcess;
                    UserContext.pContext = &Context;
                    UserContext.bWow64 = bWow64;
                    UserContext.Console = Console;
                }

                while (TRUE)
                {
                    if (!StackWalk64(MachineType, hProcess, hThread[i], &StackFrame,
                        &Context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
                        break;

                    if (!SymFromAddrW(hProcess, StackFrame.AddrPC.Offset, &Displacement64, pSymbol))
                        break;

                    *p++ = '#';

                    if (count < 10)
                    {
                        *p++ = '0' + count;
                        *p = ' ';
                    } else if (count < 100)
                    {
                        *p++ = '0' + count / 10;
                        *p = '0' + count % 10;
                    } else break;

                    ++p;
                    *p++ = ' ';

                    if (Console)
                    {
                        memcpy(p, "\x1b[34m", 5);
                        p += 5;
                    }

                    p = ulltoaddr(StackFrame.AddrPC.Offset, p, bWow64);

                    if (Console)
                    {
                        memcpy(p, "\x1b[m in \x1b[33m",
                            strlen("\x1b[m in \x1b[33m"));
                        p += strlen("\x1b[m in \x1b[33m");
                    } else
                    {
                        memcpy(p, " in ",
                            strlen(" in "));
                        p += strlen(" in ");
                    }

                    RtlUnicodeToUTF8N(p, buffer + BUFLEN - p,
                        &UTF8StringActualByteCount, pSymbol->Name, pSymbol->NameLen << 1);
                    p += UTF8StringActualByteCount;

                    if (Console)
                    {
                        memcpy(p, "\x1b[m", strlen("\x1b[m"));
                        p += strlen("\x1b[m");
                    }

                    *p++ = ' ';
                    *p++ = '(';

                    if (debug == TRUE &&
                        SymSetContext(hProcess, (PIMAGEHLP_STACK_FRAME) &StackFrame, 0))
                    {
                        Success = TRUE;
                        UserContext.p = p;
                        UserContext.DataIsLocal = FALSE;
                        UserContext.IsFirst = TRUE;
                        SymEnumSymbolsW(hProcess, 0, NULL, EnumCallbackProc, &UserContext);
                        p = UserContext.p;
                    } else Success = FALSE;

                    *p++ = ')';
                    *p++ = ' ';

                    if (debug == TRUE && SymGetLineFromAddrW64(hProcess,
                        StackFrame.AddrPC.Offset, &Displacement, &Line))
                    {
                        memcpy(p, "at ", strlen("at "));
                        temp = wcslen(Line.FileName);
                        p += strlen("at ");

                        if (Console)
                        {
                            memcpy(p, "\x1b[32m", strlen("\x1b[32m"));
                            p += strlen("\x1b[32m");
                        }

                        // Skip %dir%/
                        if (!__builtin_wmemcmp(Line.FileName, PATH, cDirLen))
                            p = FormatFileLine(Line.FileName + cDirLen + 1,
                                Line.LineNumber, temp - cDirLen - 1, p, Console);
                        else p = FormatFileLine(Line.FileName,
                            Line.LineNumber, temp, p, Console);
                        if (verbose >= 1) p = FormatSourceCode(Line.FileName, Line.LineNumber, p, verbose);
                    } else
                    {
                        DWORD Len;

                        memcpy(p, "from ", strlen("from "));
                        p += strlen("from ");

                        if (Console)
                        {
                            memcpy(p, "\x1b[32m", 5);
                            p += 5;
                        }

                        Len = GetModuleFileNameExW(hProcess,
                            (HMODULE) SymGetModuleBase64(hProcess,
                                StackFrame.AddrPC.Offset), Tmp, WBUFLEN);

                        ULONG UTF8StringActualByteCount;
                        RtlUnicodeToUTF8N(p, buffer + BUFLEN - p,
                            &UTF8StringActualByteCount, Tmp, Len << 1);
                        p += UTF8StringActualByteCount;

                        if (Console)
                        {
                            memcpy(p, "\x1b[m", 3);
                            p += 3;
                        }

                        *p++ = '\n';
                    }

                    if (Success && verbose >= 1)
                    {
                        UserContext.p = p;
                        UserContext.DataIsLocal = TRUE;
                        SymEnumSymbolsW(hProcess, 0, NULL, EnumCallbackProc, &UserContext);
                        p = UserContext.p;
                    }

                    // Release buffer
                    if (p > buffer + PAGESIZE)
                    {
                        NtWriteFile(hStdout, NULL, NULL, NULL,
                            &IoStatusBlock, buffer, p - buffer, NULL, NULL);
                        p = buffer;
                    }

                    ++count;
                }

                NtWriteFile(hStdout, NULL, NULL, NULL,
                    &IoStatusBlock, buffer, p - buffer, NULL, NULL);

                for (i = 0; i < MAX_DLL; ++i) if (BaseOfDll[i])
                    SymUnloadModule64(hProcess,
                        (DWORD64) BaseOfDll[i]);

                SymCleanup(hProcess);

                continue;

            case RIP_EVENT:
                if (verbose >= 2)
                {
                    memcpy(buffer, "RIP ", 4);
                    p = debug_ultoa(DebugEvent.dwProcessId, buffer + 4);
                    *p = 'x';
                    p = debug_ultoa(DebugEvent.dwThreadId, p + 1);
                    *p = '\n';
                    p += FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
                        DebugEvent.u.RipInfo.dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), p + 1, 256, NULL);

                    if (DebugEvent.u.RipInfo.dwType == 1)
                    {
                        memcpy(p, "Invalid data was passed to the function that failed. This caused the application to fail", 88);
                        p += 88;
                    } else if (DebugEvent.u.RipInfo.dwType == 2)
                    {
                        memcpy(p, "Invalid data was passed to the function, but the error probably will not cause the application to fail", 102);
                        p += 102;
                    } else if (DebugEvent.u.RipInfo.dwType == 3)
                    {
                        memcpy(p, "Potentially invalid data was passed to the function, but the function completed processing", 90);
                        p += 90;
                    }

                    if (DebugEvent.u.RipInfo.dwType) *p++ = '.';
                    *p = '\n';
                    NtWriteFile(hStdout, NULL, NULL, NULL,
                        &IoStatusBlock, buffer, p - buffer + 1, NULL, NULL);
                }

                break;
        }
        ContinueDebugEvent(DebugEvent.dwProcessId, DebugEvent.dwThreadId, DBG_CONTINUE);
    }
}