//
//  SPDX-License-Identifier: BSD-3-Clause
//  Copyright (c) 2024-2025 Xuan Tan. All rights reserved.
//


// Configuration
#include "include\config\lib.h"
#include "include\config\cfg.h"


// Standard Headers
#include <stdlib.h>
#include <windows.h>
#include <devioctl.h>
#include <dbghelp.h>
#include <Psapi.h>

#pragma warning(push)
#pragma warning(disable: 4464)

// Advanced Headers
#include "..\include\jeaiii_to_text.h"
#include "..\include\cvconst.h"
#include "..\include\ntdll.h"
#include "include\config\core.h"
#include "include\utils.h"
#include "include\hex.h"
#include "include\fmt.h"
#include "include\dbg.h"
#include "include\log.h"
#include "include\symbols.h"
#include "include\timeout.h"


#pragma warning(disable: 4326)
#pragma warning(disable: 5045)

int
#if defined(_M_CEE_PURE)
__clrcall
#else
WINAPI
#endif
wmain(void)
{
    wchar_t *ptr;
    DWORD ExitStatus;
    size_t temp, len;
    char buffer[BUFLEN];
    ULONG ActualByteCount;

    LONG timeout = DEFAULT_TIMEOUT;
    BOOL breakpoint = DEFAULT_BREAKPOINT,
    firstbreak = DEFAULT_FIRSTBREAK,
    dwarf = DEFAULT_DEBUG_DWARF,
    verbose = DEFAULT_VERBOSE,
    output = DEFAULT_OUTPUT,
    start = DEFAULT_START,
    help = FALSE;

    char *p = buffer;
    PPEB pPeb = GetCurrentPeb();
    PZWRTL_USER_PROCESS_PARAMETERS pProcessParameters =
        (PZWRTL_USER_PROCESS_PARAMETERS) pPeb->ProcessParameters;
    len = pProcessParameters->CommandLine.Length >> 1;
    wchar_t *pCmdLine = wmemchr(pProcessParameters->CommandLine.Buffer, ' ', len);
    wchar_t *pNext = pCmdLine;

    if (pCmdLine)
    {
        len -= pCmdLine - pProcessParameters->CommandLine.Buffer;
        // Modified for processing command-line arguments
        *(pCmdLine + len) = ' ';

        while (TRUE)
        {
            while (*pNext == ' ') ++pNext; // Skip spaces

            if (*pNext != '/' || pCmdLine + len < pNext) break;

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

                    break;

                case 'D':
                case 'd':
                    if (*(pNext + 2) == ' ')
                    {
                        dwarf = FALSE;
                        pNext += 3;
                        continue;
                    }

                    break;

                case 'G':
                case 'g':
                    if (*(pNext + 2) == ' ')
                    {
                        dwarf = MINGW_KEEP;
                        pNext += 3;
                        continue;
                    } else if (*(pNext + 2) == '+' && *(pNext + 3) == ' ')
                    {
                        dwarf = MINGW_NOKEEP;
                        pNext += 4;
                        continue;
                    }

                    break;

                case 'O':
                case 'o':
                    if (*(pNext + 2) == ' ')
                    {
                        output = FALSE;
                        pNext += 3;
                        continue;
                    }

                    break;

                case 'S':
                case 's':
                    if (*(pNext + 2) == ' ')
                    {
                        start = TRUE;
                        pNext += 3;
                        continue;
                    }

                    break;

                case 'T':
                case 't':
                    while (*pNext == ' ') ++pNext; // Skip spaces

                    pNext += 2;
                    ptr = pNext;
                    temp = pCmdLine + len + 1 - pNext;

                    if (temp <= 0)
                    {
                        ExitStatus = ERROR_BAD_ARGUMENTS;

                        memcpy(p, VALUE_EXPECTED,
                            strlen(VALUE_EXPECTED));
                        p += strlen(VALUE_EXPECTED);
                        *p++ = *ptr;
                        *p++ = *(ptr + 1);
                        *p++ = '\'';
                        *p++ = '\n';
                    } else if ((timeout = process_timeout(pNext, &pNext, temp)) > VALID_TIMEOUT)
                    {
                        memcpy(p, _TIMEOUT_INVALID, strlen(_TIMEOUT_INVALID));
                        p += strlen(_TIMEOUT_INVALID);
                        *p++ = *ptr;
                        *p++ = *(ptr + 1);
                        memcpy(p, TIMEOUT_INVALID_, strlen(TIMEOUT_INVALID_));
                        p += strlen(TIMEOUT_INVALID_);
                    }

                    continue;

                case 'V':
                case 'v':
                    if (*(pNext + 2) == ' ')
                    {
                        verbose = 2;
                        pNext += 3;
                        continue;
                    } else if (*(pNext + 2) >= '0' &&
                        *(pNext + 2) <= '2' && *(pNext + 3) == ' ')
                    {
                        verbose = *(pNext + 2) - '0';
                        pNext += 4;
                        continue;
                    }

                    break;

                case '?':
                    if (*(pNext + 2) == ' ')
                    {
                        help = TRUE;
                        pNext += 3;
                        continue;
                    }

                    break;
            }

            ExitStatus = ERROR_INVALID_PARAMETER;

            memcpy(p, _INVALID_ARGUMENT,
                strlen(_INVALID_ARGUMENT));
            p += strlen(_INVALID_ARGUMENT);

            temp = wmemchr(pNext, ' ',
                pCmdLine + len + 1 - pNext) - pNext;

            RtlUnicodeToUTF8N(p, buffer + BUFLEN - p,
                &ActualByteCount, pNext, temp << 1);
            p += ActualByteCount;
            pNext += temp + 1;

            memcpy(p, INVALID_ARGUMENT_, 3);
            p += 3;
        }
    }

    if (help)
    { // help message
        ExitStatus = EXIT_SUCCESS;
        memcpy(p, HELP + 16, strlen(HELP) - 16);
        p += strlen(HELP) - 16;
    } else if (!pCmdLine || pCmdLine + len < pNext)
    { // No executable specified
        ExitStatus = ERROR_BAD_ARGUMENTS;
        memcpy(p, HELP, 65);
        p += 65;
    }

    BOOL Console;
    IO_STATUS_BLOCK IoStatusBlock;

    // GetCurrentPeb()->ProcessParameters->StandardOutput
    HANDLE hStdout = GetCurrentPeb()->ProcessParameters->Reserved2[3];

    IsConsoleHandle(hStdout, &Console);
    if (Console)
    {
        SetConsoleOutputCP(65001);  // CP_UTF8
        SetConsoleMode(hStdout, ENABLE_PROCESSED_OUTPUT |
            ENABLE_WRAP_AT_EOL_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }

    if (p != buffer)
    {
        NtWriteFile(hStdout, NULL, NULL, NULL,
            &IoStatusBlock, buffer, p - buffer, NULL, NULL);
        return ExitStatus;
    }

    wchar_t ApplicationName[WBUFLEN];

    ptr = wmemchr(pNext, ' ',
        pCmdLine + len + 1 - pNext);
    *ptr = '\0';

    *(pCmdLine + len) = '\0';

    if (!SearchPathW(NULL, pNext, EXTENSION,
        sizeof(ApplicationName) >> 1, ApplicationName, NULL))
    { // Check if executable exists
        PMESSAGE_RESOURCE_ENTRY Entry;

        LookupSystemMessage(pPeb->Ldr, ERROR_FILE_NOT_FOUND, GetCurrentLangID(), &Entry);
        // Convert error message to UTF-8
        RtlUnicodeToUTF8N(buffer, BUFLEN, &ActualByteCount,
            (PCWSTR) Entry->Text, Entry->Length - 8);
        NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock,
            buffer, ActualByteCount, NULL, NULL);
        return ERROR_FILE_NOT_FOUND;
    }

    DWORD is_64bit; // Is 64-bit application

    if (!GetBinaryTypeW(ApplicationName, &is_64bit) ||
        (is_64bit != SCS_32BIT_BINARY && is_64bit != SCS_64BIT_BINARY))
    { // Check if executable format (x86-64)
        wchar_t *pos;
        PMESSAGE_RESOURCE_ENTRY Entry;

        LookupSystemMessage(pPeb->Ldr, ERROR_BAD_EXE_FORMAT, GetCurrentLangID(), &Entry);

        // Convert error message to UTF-8
        if (*Entry->Text == '%')
        {
            pos = (wchar_t*) Entry->Text;
            p = buffer;
        }
        else
        {
            // Should - 8 and () >> 1
            pos = wmemchr((wchar_t*) Entry->Text, '%', Entry->Length);
            RtlUnicodeToUTF8N(buffer, BUFLEN, &ActualByteCount,
                (PCWSTR) Entry->Text, pos - (wchar_t*) Entry->Text);
            p = buffer + ActualByteCount;
        }

        // Convert filename to UTF-8
        RtlUnicodeToUTF8N(p, buffer + BUFLEN - p, &ActualByteCount,
            pNext, (ptr - pNext) << 1);
        p += ActualByteCount;

        pos = wmemchr(pos + 1, '1', Entry->Length) + 1;
        // Convert error message to UTF-8
        RtlUnicodeToUTF8N(p, buffer + BUFLEN - p, &ActualByteCount,
            pos, Entry->Length - 8 - ((pos - (wchar_t*) Entry->Text) << 1));
        p += ActualByteCount;

        NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock,
            buffer, p - buffer, NULL, NULL);
        return ERROR_BAD_EXE_FORMAT;
    }

    HANDLE hFile[MAX_DLL];
    HANDLE hJob, hProcess;
    STARTUPINFOW startupInfo;
    HANDLE hThread[MAX_THREAD];
    PROCESS_INFORMATION processInfo;
    JOBOBJECT_BASIC_LIMIT_INFORMATION jobInfo;
    LPVOID BaseOfDll[MAX_DLL] = {};

    if (pCmdLine + len != ptr) *ptr = ' ';

    startupInfo.cb = sizeof(startupInfo);
    startupInfo.lpReserved = NULL;
    startupInfo.lpDesktop = NULL;
    startupInfo.lpTitle = NULL;
    startupInfo.dwFlags = 0;
    startupInfo.cbReserved2 = 0;
    startupInfo.lpReserved2 = NULL;

    DBGUI_WAIT_STATE_CHANGE StateChange;
    OBJECT_ATTRIBUTES ObjectAttributes = {sizeof(OBJECT_ATTRIBUTES), NULL, NULL, OBJ_OPENIF, NULL, NULL};

    // Avoid resources leak
    jobInfo.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    NtCreateJobObject(&hJob, JOB_OBJECT_ALL_ACCESS, &ObjectAttributes);
    NtSetInformationJobObject(hJob, JobObjectBasicLimitInformation, &jobInfo, sizeof(jobInfo));

    CreateProcessW(ApplicationName, pNext, NULL, NULL, FALSE,
        start ? CREATIONFLAGS | DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE
              : CREATIONFLAGS | DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_PROCESS_GROUP,
        pProcessParameters->Environment, pProcessParameters->CurrentDirectory.DosPath.Buffer, &startupInfo, &processInfo);
    NtAssignProcessToJobObject(hJob, processInfo.hProcess);
    DbgWaitForDebugEvent(&StateChange, NULL);

    if (!dwarf)
    {
        NtClose(processInfo.hThread);
        NtClose(processInfo.hProcess);
        hFile[0] = StateChange.StateInfo.CreateProcessInfo.NewProcess.FileHandle;
        BaseOfDll[0] = StateChange.StateInfo.CreateProcessInfo.NewProcess.BaseOfImage;
        hProcess = StateChange.StateInfo.CreateProcessInfo.HandleToProcess;
        hThread[0] = StateChange.StateInfo.CreateProcessInfo.HandleToThread;
    } else
    {
        NtClose(StateChange.StateInfo.CreateProcessInfo.NewProcess.FileHandle);
        hThread[0] = processInfo.hThread;
        hProcess = processInfo.hProcess;
    }

    if (verbose >= 2) DbgTraceEvent(&StateChange, CREATE_PROCESS, strlen(CREATE_PROCESS), hStdout);

    DbgContinueDebugEvent(&StateChange, DBG_CONTINUE);

    // x86 process / MinGW64 G++ trigger
    // more than one breakpoints at start-up
    if ((is_64bit && dwarf) || !is_64bit) --firstbreak;

    DWORD i;
    wchar_t PATH[WBUFLEN];
    UNICODE_STRING Path, FileName, FoundFile;
    DWORD dwThreadId[MAX_THREAD] = {};

    *PATH = '\0';
    dwThreadId[0] = HandleToUlong(StateChange.AppClientId.UniqueThread);

    while (TRUE)
    {
        DbgWaitForDebugEvent(&StateChange, NULL);

        switch (StateChange.NewState)
        {
        case DbgLoadDllStateChange:
            if (verbose >= 2) DbgTraceModule(StateChange.StateInfo.LoadDll.FileHandle, LOAD_DLL, strlen(LOAD_DLL), hStdout);

            // Find storage position
            for (i = 0; i < MAX_DLL; ++i) if (!BaseOfDll[i])
            {
                hFile[i] = StateChange.StateInfo.LoadDll.FileHandle;
                BaseOfDll[i] = StateChange.StateInfo.LoadDll.BaseOfDll;
                break;
            }

            break;

        case DbgUnloadDllStateChange:
            // Find specific DLL
            for (i = 0; i < MAX_DLL; ++i) if (StateChange.StateInfo.UnloadDll.BaseAddress == BaseOfDll[i])
            {
                if (verbose >= 2) DbgTraceModule(hFile[i], UNLOAD_DLL, strlen(UNLOAD_DLL), hStdout);

                NtClose(hFile[i]);
                BaseOfDll[i] = 0;
                break;
            }

            break;

        case DbgCreateThreadStateChange:
            if (verbose >= 2) DbgTraceEvent(&StateChange, CREATE_THREAD, strlen(CREATE_THREAD), hStdout);

            // Find storage position
            for (i = 0; i < MAX_THREAD; ++i) if (!dwThreadId[i])
            {
                hThread[i] = StateChange.StateInfo.CreateThread.HandleToThread;
                dwThreadId[i] = HandleToUlong(StateChange.AppClientId.UniqueThread);
                break;
            }

            break;

        case DbgExitThreadStateChange:
            if (verbose >= 2) DbgTraceEvent(&StateChange, EXIT_THREAD, strlen(EXIT_THREAD), hStdout);

            // Find specific thread
            for (i = 0; i < MAX_THREAD; ++i) if (dwThreadId[i] == HandleToUlong(StateChange.AppClientId.UniqueThread))
            {
                dwThreadId[i] = 0;
                break;
            }

            break;

        case DbgExitProcessStateChange:
            if (verbose >= 2) DbgTraceEvent(&StateChange, EXIT_PROCESS, strlen(EXIT_PROCESS), hStdout);

            if (dwarf)
            {
                NtClose(hProcess);
                NtClose(hThread[0]);
            }

            for (i = 0; i < MAX_DLL; ++i)
                if (BaseOfDll[i]) NtClose(hFile[i]);

            if (timeout)
                WaitForInputOrTimeout(pProcessParameters->StandardInput, hStdout, timeout, Console);

            DbgContinueDebugEvent(&StateChange, DBG_CONTINUE);
            NtClose(hJob);
            return EXIT_SUCCESS;

        case DbgExceptionStateChange:
        case DbgBreakpointStateChange:
        case DbgSingleStepStateChange:
            if (StateChange.StateInfo.Exception.ExceptionRecord.ExceptionCode == DBG_PRINTEXCEPTION_WIDE_C ||
                StateChange.StateInfo.Exception.ExceptionRecord.ExceptionCode == DBG_PRINTEXCEPTION_C)
            {
                if (verbose >= 2) DbgTraceEvent(&StateChange, OUTPUT_DEBUG, strlen(OUTPUT_DEBUG), hStdout);
                if (output == TRUE) DbgProcessODS(&StateChange, hProcess, hStdout);
                break;
            } else if (StateChange.StateInfo.Exception.ExceptionRecord.ExceptionCode == DBG_RIPEXCEPTION)
            {
                if (verbose >= 2) DbgTraceEvent(&StateChange, RIP, strlen(RIP), hStdout);
                DbgProcessRIP(&StateChange, pPeb->Ldr, hStdout);
                break;
            }

            // Ignore thread naming exception
            if (StateChange.StateInfo.Exception.ExceptionRecord.ExceptionCode == 0x406D1388) // MS_VC_EXCEPTION
                break;

            // Ignore first-chance breakpoints
            if ((StateChange.StateInfo.Exception.ExceptionRecord.ExceptionCode == 0x80000003 || // STATUS_BREAKPOINT
                StateChange.StateInfo.Exception.ExceptionRecord.ExceptionCode == 0x4000001F) && // STATUS_WX86_BREAKPOINT
                ((breakpoint == FALSE) || (breakpoint == TRUE && ++firstbreak <= 1)))
                break;

            // Ignore other first change exceptions
            if (StateChange.StateInfo.Exception.FirstChance)
            {
                DbgContinueDebugEvent(&StateChange, DBG_EXCEPTION_NOT_HANDLED);
                continue;
            }

            // Find thread where exception occured
            for (i = 0; i < MAX_THREAD; ++i) if (HandleToUlong(StateChange.AppClientId.UniqueThread) == dwThreadId[i])
                break;

            memcpy(buffer, THREAD_NUMBER, strlen(THREAD_NUMBER));
            if (HandleToUlong(StateChange.AppClientId.UniqueThread) == dwThreadId[i])
            {
                buffer[strlen(THREAD_NUMBER)] = '0' + (i + 1) / 10;
                buffer[strlen(THREAD_NUMBER) + 1] = '0' + (i + 1) % 10;
                p = buffer + strlen(THREAD_NUMBER) + 2;
            } else p = jeaiii::to_ascii_chars(buffer + strlen(THREAD_NUMBER) - 1, HandleToUlong(StateChange.AppClientId.UniqueThread));

            wchar_t Tmp[WBUFLEN];

            memcpy(p, THREAD_TRIGGERD, strlen(THREAD_TRIGGERD));
            p += strlen(THREAD_TRIGGERD);
            p = _ultoa16u(StateChange.StateInfo.Exception.ExceptionRecord.ExceptionCode, p);
            *p++ = '\n';

            PMESSAGE_RESOURCE_ENTRY Entry;

            // Add messages for STATUS_APPLICATION_HANG, STATUS_CPP_EH_EXCEPTION, STATUS_CLR_EXCEPTION

            if (NT_SUCCESS(LookupNtdllMessage(pPeb->Ldr,
                StateChange.StateInfo.Exception.ExceptionRecord.ExceptionCode, GetCurrentLangID(), &Entry)))
            {
                if (Console)
                {
                    memcpy(p, CONSOLE_NRED_FORMAT, strlen(CONSOLE_NRED_FORMAT));
                    p += strlen(CONSOLE_NRED_FORMAT);
                }

                // Convert error message to UTF-8
                RtlUnicodeToUTF8N(p, buffer + BUFLEN - p,
                    &ActualByteCount, (PCWSTR) Entry->Text, Entry->Length - 8);
                p += ActualByteCount;

                if (Console)
                {
                    memcpy(p, CONSOLE_DEFAULT_FORMAT, strlen(CONSOLE_DEFAULT_FORMAT));
                    p += strlen(CONSOLE_DEFAULT_FORMAT);
                }
            }

            if (dwarf && !*PATH)
            {
                SIZE_T ReturnLength;
                RtlQueryEnvironmentVariable(pProcessParameters->Environment, PATHENV,
                    wcslen(PATHENV), PATH, WBUFLEN, &ReturnLength);
                Path.Length = ReturnLength << 1;
                Path.Buffer = PATH;
                FileName.Length = wcslen(GDB_EXE) << 1;
                FileName.MaximumLength = (wcslen(GDB_EXE) << 1) + 2;
                FileName.Buffer = (PWSTR) GDB_EXE;
                FoundFile.MaximumLength = sizeof(ApplicationName) + 2;
                FoundFile.Buffer = ApplicationName;
            }

            if (dwarf && NT_SUCCESS(RtlDosSearchPath_Ustr(RTL_DOS_SEARCH_PATH_FLAG_DISALLOW_DOT_RELATIVE_PATH_SEARCH,
                &Path, &FileName, NULL, &FoundFile, NULL, NULL, NULL, NULL)))
            { // Check if executable exists
                NtWriteFile(hStdout, NULL, NULL, NULL,
                    &IoStatusBlock, buffer, p - buffer, NULL, NULL);
                NtSuspendProcess(hProcess);
                DbgContinueDebugEvent(&StateChange, DBG_REPLY_LATER);
                DebugActiveProcessStop(HandleToUlong(StateChange.AppClientId.UniqueProcess));

                HANDLE hTemp;
                UNICODE_STRING String;
                LARGE_INTEGER ByteOffset;
                wchar_t CommandLine[(sizeof(GDB_COMMAND_LINE) >> 1) - 1 + MAX_PATH];

                memcpy(CommandLine, GDB_COMMAND_LINE, wcslen(GDB_COMMAND_LINE) << 1);
                RtlQueryEnvironmentVariable(pProcessParameters->Environment, TMPENV,
                    wcslen(TMPENV), &CommandLine[wcslen(GDB_COMMAND_LINE)],
                    (sizeof(CommandLine) >> 1) - wcslen(GDB_COMMAND_LINE), &temp);

                // Ensure correct DOS path
                if (CommandLine[wcslen(GDB_COMMAND_LINE) - 1 + temp] != '\\')
                {
                    CommandLine[wcslen(GDB_COMMAND_LINE) + temp] = '\\';
                    ++temp;
                }

                memcpy(&CommandLine[wcslen(GDB_COMMAND_LINE) + temp], W64DBG, wcslen(W64DBG) << 1);
                String.Length = 8 + (wcslen(W64DBG) << 1) + (temp << 1);
                String.Buffer = &CommandLine[wcslen(GDB_COMMAND_LINE) - 4];

                OBJECT_ATTRIBUTES ObjectAttributes = {sizeof(OBJECT_ATTRIBUTES), NULL, &String, OBJ_CASE_INSENSITIVE, NULL, NULL};

                NtCreateFile(&hTemp, FILE_WRITE_DATA | SYNCHRONIZE, &ObjectAttributes,
                    &IoStatusBlock, NULL, FILE_ATTRIBUTE_NOT_CONTENT_INDEXED, 0, FILE_OPEN_IF,
                    FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

                // First time run
                if (IoStatusBlock.Information == FILE_CREATED)
                    NtWriteFile(hTemp, NULL, NULL, NULL, &IoStatusBlock,
                        (PVOID) GDB_DEFAULT, strlen(GDB_DEFAULT), NULL, NULL);

                p = jeaiii::to_ascii_chars(buffer, HandleToUlong(StateChange.AppClientId.UniqueProcess));

                if (dwarf == MINGW_NOKEEP)
                {
                    memcpy(p, GDB_PRESERVE, strlen(GDB_PRESERVE));
                    p += strlen(GDB_PRESERVE);
                } else
                {
                    memcpy(p, GDB_STYLE, strlen(GDB_STYLE));
                    p += strlen(GDB_STYLE);
                }

                if (verbose >= 1)
                {
                    memcpy(p, GDB_FRAME_ARG, strlen(GDB_FRAME_ARG));
                    p += strlen(GDB_FRAME_ARG);
                }

                memcpy(p, GDB_CONTINUE, strlen(GDB_CONTINUE));
                p += strlen(GDB_CONTINUE);

                if (verbose >= 1)
                {
                    *p++ = ' ';
                    *p++ = 'f';
                }

                ByteOffset.QuadPart = strlen(GDB_DEFAULT);
                NtWriteFile(hTemp, NULL, NULL, NULL, &IoStatusBlock,
                    buffer, p - buffer, &ByteOffset, NULL);

                ByteOffset.QuadPart +=  p - buffer;
                NtSetInformationFile(hTemp, &IoStatusBlock, &ByteOffset,
                    sizeof(LARGE_INTEGER), FileEndOfFileInformation);
                NtClose(hTemp);

                if (dwarf == MINGW_KEEP && !timeout)
                    memcpy(&CommandLine[wcslen(GDB_COMMAND_LINE) + temp + 6], GDB_BATCH, wcslen(GDB_BATCH) << 1);

                CreateProcessW(ApplicationName, CommandLine, NULL, NULL,
                    FALSE, CREATIONFLAGS, pProcessParameters->Environment,
                    pProcessParameters->CurrentDirectory.DosPath.Buffer, &startupInfo, &processInfo);
                NtAssignProcessToJobObject(hJob, processInfo.hProcess);
                NtClose(processInfo.hThread);
                NtClose(hThread[0]);

                for (i = 0; i < MAX_DLL; ++i)
                    if (BaseOfDll[i]) NtClose(hFile[i]);

                DWORD_PTR ProcDbgPt;
                LARGE_INTEGER DelayInterval;

                // Convert seconds to 100-nanosecond units
                // (negative for relative time)
                DelayInterval.QuadPart = -MiliSecToUnits(LATENCY);

                while (TRUE)
                { // Wait until GDB attachs
                    NtDelayExecution(FALSE, &DelayInterval);
                    NtQueryInformationProcess(hProcess, ProcessDebugPort,
                        &ProcDbgPt, sizeof(ProcDbgPt), NULL);
                    if (ProcDbgPt) break;
                }

                NtResumeProcess(hProcess);
                NtClose(hProcess);

                if (timeout)
                { // Quit GDB first
                    if (Console)
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

                        WriteConsoleInputW(pProcessParameters->StandardInput,
                            InputRecord, 2, &dwWriten);
                    } else NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock,
                        (PVOID) GDB_QUIT, strlen(GDB_QUIT), NULL, NULL);
                }

                NtWaitForSingleObject(processInfo.hProcess, FALSE, NULL);
                NtClose(processInfo.hProcess);

                if (verbose >= 2) DbgTraceEvent(&StateChange, EXIT_PROCESS, strlen(EXIT_PROCESS), hStdout);

                if (timeout) WaitForInputOrTimeout(
                    pProcessParameters->StandardInput, hStdout, timeout, Console);

                NtClose(hJob);
                return EXIT_SUCCESS;
            }

            if (HandleToUlong(StateChange.AppClientId.UniqueThread) != dwThreadId[i])
            {
                NtWriteFile(hStdout, NULL, NULL, NULL,
                    &IoStatusBlock, buffer, p - buffer, NULL, NULL);
                continue;
            }

            SymSetOptions(SYMOPTIONS);
            SymInitializeW(hProcess, NULL, FALSE);

            for (DWORD j = 0; j < MAX_DLL; ++j) if (BaseOfDll[j])
                SymLoadModuleExW(hProcess,
                    hFile[j],
                    NULL,
                    NULL,
                    (DWORD64) BaseOfDll[j],
                    GetModuleSize(hFile[j]),
                    NULL,
                    0);

            CONTEXT Context;
            DWORD MachineType;
            STACKFRAME_EX StackFrame;

            if (is_64bit)
            {
                MachineType = IMAGE_FILE_MACHINE_AMD64;
                Context.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
                NtGetContextThread(hThread[i], &Context);
                StackFrame.AddrPC.Offset = Context.Rip;
                StackFrame.AddrFrame.Offset = Context.Rbp;
                StackFrame.AddrStack.Offset = Context.Rsp;
            } else
            {
                MachineType = IMAGE_FILE_MACHINE_I386;
                ((PWOW64_CONTEXT) &Context)->ContextFlags = WOW64_CONTEXT_CONTROL | WOW64_CONTEXT_INTEGER;
                NtQueryInformationThread(hThread[i], ThreadWow64Context,
                    &Context, sizeof(WOW64_CONTEXT), NULL);
                StackFrame.AddrPC.Offset = ((PWOW64_CONTEXT) &Context)->Eip;
                StackFrame.AddrFrame.Offset = ((PWOW64_CONTEXT) &Context)->Ebp;
                StackFrame.AddrStack.Offset = ((PWOW64_CONTEXT) &Context)->Esp;
            }

            StackFrame.AddrPC.Mode = AddrModeFlat;
            StackFrame.AddrFrame.Mode = AddrModeFlat;
            StackFrame.AddrStack.Mode = AddrModeFlat;
            // StackFrame.StackFrameSize = sizeof(StackFrame);

            DWORD count;
            DWORD DirLen;
            BOOL Success;
            DWORD Displacement;
            PSYMBOL_INFOW pSymInfo;
            IMAGEHLP_LINEW64 Line;
            USERCONTEXT UserContext;
            char Symbol[sizeof(SYMBOL_INFO) + (MAX_SYM_NAME << 1)];

            pSymInfo = (PSYMBOL_INFOW) Symbol;
            Line.SizeOfStruct = sizeof(Line);
            pSymInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
            pSymInfo->MaxNameLen = MAX_SYM_NAME;

            count = 0;
            UserContext.hProcess = hProcess;
            UserContext.pContext = &Context;
            UserContext.pBase = &StackFrame.AddrFrame.Offset;
            UserContext.is_64bit = is_64bit;
            UserContext.Console = Console;
            DirLen = pProcessParameters->CurrentDirectory.DosPath.Length;

            while (TRUE)
            {
                StackFrame.InlineFrameContext = INLINE_FRAME_CONTEXT_IGNORE;

                if (!StackWalk2(MachineType, hProcess, hThread[i], &StackFrame, &Context,
                    ReadMemoryRoutineLocal, NULL, NULL, NULL, NULL, SYM_STKWALK_DEFAULT)) break;

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
                    memcpy(p, CONSOLE_BLUE_FORMAT, strlen(CONSOLE_BLUE_FORMAT));
                    p += strlen(CONSOLE_BLUE_FORMAT);
                }

                p = __ui64toaddr(StackFrame.AddrPC.Offset, p, is_64bit);

                if (Console)
                {
                    memcpy(p, EXCEPTION_IN, strlen(EXCEPTION_IN));
                    p += strlen(EXCEPTION_IN);
                } else
                {
                    memcpy(p, EXCEPTION_IN + 3, 4);
                    p += 4;
                }

                //
                //  StackFrame.AddrPC.Offset stores the return address rather than the caller's address.
                //  Adjust StackFrame.AddrPC.Offset by 'count' to resolve the caller's address accurately.
                //

                if (SymFromInlineContextW(hProcess, StackFrame.AddrPC.Offset - count,
                    INLINE_FRAME_CONTEXT_IGNORE, NULL, pSymInfo))
                {
                    RtlUnicodeToUTF8N(p, buffer + BUFLEN - p,
                        &ActualByteCount, pSymInfo->Name, pSymInfo->NameLen << 1);
                    p += ActualByteCount;
                } else
                {
                    *p++ = '?';
                    *p++ = '?';
                }

                if (Console)
                {
                    memcpy(p, CONSOLE_DEFAULT_FORMAT,
                        strlen(CONSOLE_DEFAULT_FORMAT));
                    p += strlen(CONSOLE_DEFAULT_FORMAT);
                }

                *p++ = ' ';
                *p++ = '(';

                if (SymSetContext(hProcess, (PIMAGEHLP_STACK_FRAME) &StackFrame, &Context))
                {
                    Success = TRUE;
                    UserContext.p = p;
                    UserContext.DataIsParam = TRUE;
                    SymEnumSymbolsExW(hProcess, 0, NULL, EnumCallbackProc, &UserContext, SYMENUM_OPTIONS_DEFAULT);
                    if (p != UserContext.p) p = UserContext.p - 2;
                } else Success = FALSE;

                *p++ = ')';
                *p++ = ' ';

                // Without &Displacement => undefined behavior
                if (SymGetLineFromInlineContextW(hProcess, StackFrame.AddrPC.Offset - count,
                    INLINE_FRAME_CONTEXT_IGNORE, NULL, &Displacement, &Line))
                {
                    memcpy(p, EXCEPTION_AT, strlen(EXCEPTION_AT));
                    temp = wcslen(Line.FileName) << 1;
                    p += strlen(EXCEPTION_AT);

                    if (Console)
                    {
                        memcpy(p, CONSOLE_GREEN_FORMAT, strlen(CONSOLE_GREEN_FORMAT));
                        p += strlen(CONSOLE_GREEN_FORMAT);
                    }

                    // Skip %dir%/
                    if (temp > DirLen && !memcmp(Line.FileName,
                            pProcessParameters->CurrentDirectory.DosPath.Buffer, DirLen))
                        p = FormatFileLine(Line.FileName + (DirLen >> 1),
                            Line.LineNumber, temp - DirLen, buffer + BUFLEN - p, p, Console);
                    else p = FormatFileLine(Line.FileName,
                        Line.LineNumber, temp, buffer + BUFLEN - p, p, Console);
                    if (verbose >= 1) p = FormatSourceCode(pPeb->Ldr, Line.FileName,
                        Line.LineNumber, temp, buffer + BUFLEN - p, p);
                } else
                {
                    memcpy(p, EXCEPTION_FROM, strlen(EXCEPTION_FROM));
                    p += strlen(EXCEPTION_FROM);

                    if (Console)
                    {
                        memcpy(p, CONSOLE_GREEN_FORMAT, strlen(CONSOLE_GREEN_FORMAT));
                        p += strlen(CONSOLE_GREEN_FORMAT);
                    }

                    len = GetModuleFileNameExW(hProcess,
                        (HMODULE) SymGetModuleBase64(hProcess, StackFrame.AddrPC.Offset), Tmp, WBUFLEN);
                    RtlUnicodeToUTF8N(p, buffer + BUFLEN - p,
                        &ActualByteCount, Tmp, len << 1);
                    p += ActualByteCount;

                    if (Console)
                    {
                        memcpy(p, CONSOLE_DEFAULT_FORMAT,
                            strlen(CONSOLE_DEFAULT_FORMAT));
                        p += strlen(CONSOLE_DEFAULT_FORMAT);
                    }

                    *p++ = '\n';
                }

                if (Success && verbose >= 1)
                {
                    UserContext.p = p;
                    UserContext.DataIsParam = FALSE;
                    SymEnumSymbolsExW(hProcess, 0, NULL, EnumCallbackProc, &UserContext, SYMENUM_OPTIONS_DEFAULT);
                    p = UserContext.p;
                }

                // Release buffer
                if (p > buffer + (sizeof(buffer) >> 1))
                {
                    NtWriteFile(hStdout, NULL, NULL, NULL,
                        &IoStatusBlock, buffer, p - buffer, NULL, NULL);
                    p = buffer;
                }

                ++count;
            }

            NtWriteFile(hStdout, NULL, NULL, NULL,
                &IoStatusBlock, buffer, p - buffer, NULL, NULL);

            SymCleanup(hProcess);
            NtTerminateProcess(hProcess, StateChange.StateInfo.Exception.ExceptionRecord.ExceptionCode);
            break;
        }

        DbgContinueDebugEvent(&StateChange, DBG_CONTINUE);
    }
}

#pragma warning(pop)