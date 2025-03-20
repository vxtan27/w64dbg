// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

// Config
#include "include/config/crt.h"

#pragma warning(push)
#pragma warning(disable: 4005)

#include <stdlib.h>
#include <phnt_windows.h>
#include <phnt.h>

#include <devioctl.h>
#include <dbghelp.h>
#include <cvconst.h>
#include <psapi.h>

#pragma warning(pop)

#include "include/conversion/hex.h"
#include "include/conversion/int_to_chars.h"

#include "include/config/core.h"
#include "include/exception.h"
#include "include/utils.h"
#include "include/fmt.h"
#include "include/dbg.h"
#include "include/log.h"
#include "include/symbols.h"
#include "include/timeout.h"

int
#if defined(_M_CEE_PURE)
__clrcall
#else
WINAPI
#endif
wmain(void) {
    wchar_t *ptr;
    DWORD ExitStatus;
    size_t temp, len;
    char buffer[BUFLEN];
    ULONG ActualByteCount;

    DWORD timeout = DEFAULT_TIMEOUT;
    BOOL breakpoint = DEFAULT_BREAKPOINT,
    firstbreak = DEFAULT_FIRSTBREAK,
    verbose = DEFAULT_VERBOSE,
    output = DEFAULT_OUTPUT,
    start = DEFAULT_START,
    help = FALSE;

    char *p = buffer;
    PUNICODE_STRING pCommandLine = GetCommandLine();
    len = pCommandLine->Length >> 1;
    wchar_t *pCmdLine = wmemchr(pCommandLine->Buffer, ' ', len);
    wchar_t *pNext = pCmdLine;

    if (pCmdLine) {
        len -= pCmdLine - pCommandLine->Buffer;
        // Modified for processing command-line arguments
        *(pCmdLine + len) = ' ';

        while (TRUE) {
            while (*pNext == ' ') ++pNext; // Skip spaces

            if (*pNext != '/' || pCmdLine + len < pNext) break;

            switch (*(pNext + 1)) {
            case 'B':
            case 'b':
                if (*(pNext + 2) == ' ') {
                    breakpoint = FALSE;
                    pNext += 3;
                    continue;
                }

                break;

            case 'O':
            case 'o':
                if (*(pNext + 2) == ' ') {
                    output = FALSE;
                    pNext += 3;
                    continue;
                }

                break;

            case 'S':
            case 's':
                if (*(pNext + 2) == ' ') {
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

                if (temp <= 0) {
                    ExitStatus = ERROR_BAD_ARGUMENTS;

                    memcpy(p, VALUE_EXPECTED,
                        strlen(VALUE_EXPECTED));
                    p += strlen(VALUE_EXPECTED);
                    *p++ = *ptr;
                    *p++ = *(ptr + 1);
                    *p++ = '\'';
                    *p++ = '\n';
                } else if ((timeout = process_timeout(pNext, &pNext, temp)) > VALID_TIMEOUT) {
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
                if (*(pNext + 2) == ' ') {
                    verbose = 2;
                    pNext += 3;
                    continue;
                } else if (*(pNext + 2) >= '0' &&
                    *(pNext + 2) <= '2' && *(pNext + 3) == ' ') {
                    verbose = *(pNext + 2) - '0';
                    pNext += 4;
                    continue;
                }

                break;

            case '?':
                if (*(pNext + 2) == ' ') {
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

            p += ConvertUnicodeToUTF8(pNext, temp << 1, p, buffer + BUFLEN - p);
            pNext += temp + 1;

            memcpy(p, INVALID_ARGUMENT_, 3);
            p += 3;
        }
    }

    if (help) { // help message
        ExitStatus = EXIT_SUCCESS;
        memcpy(p, HELP + 16, strlen(HELP) - 16);
        p += strlen(HELP) - 16;
    } else if (!pCmdLine || pCmdLine + len < pNext) { // No executable specified
        ExitStatus = ERROR_BAD_ARGUMENTS;
        memcpy(p, HELP, 65);
        p += 65;
    }

    BOOL bConsole;
    IO_STATUS_BLOCK IoStatus;
    HANDLE hStdout = GetStandardOutput();

    IsConsoleHandle(hStdout, &bConsole);
    if (bConsole) {
        SetConsoleOutputCP(65001);  // CP_UTF8
        SetConsoleMode(hStdout, ENABLE_PROCESSED_OUTPUT |
            ENABLE_WRAP_AT_EOL_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }

    if (p != buffer) {
        WriteDataA(hStdout, buffer, p - buffer, bConsole);
        return ExitStatus;
    }

    wchar_t ApplicationName[WBUFLEN];

    ptr = wmemchr(pNext, ' ',
        pCmdLine + len + 1 - pNext);
    *ptr = '\0';

    *(pCmdLine + len) = '\0';

    if (!SearchPathW(NULL, pNext, EXTENSION,
        sizeof(ApplicationName) >> 1, ApplicationName, NULL)) { // Check if executable exists
        PMESSAGE_RESOURCE_ENTRY MessageEntry;

        LookupSystemMessage(ERROR_FILE_NOT_FOUND, LANG_USER_DEFAULT, &MessageEntry);
        WriteDataW(hStdout, GetMessageEntryText(MessageEntry),
            GetMessageEntryLength(MessageEntry) >> 1, bConsole);
        return ERROR_FILE_NOT_FOUND;
    }

    DWORD b64bit; // Is 64-bit application

    // Check if executable format (x86-64)
    if (!GetBinaryTypeW(ApplicationName, &b64bit) ||
        (b64bit != SCS_32BIT_BINARY && b64bit != SCS_64BIT_BINARY)) {
        wchar_t *pos;
        PMESSAGE_RESOURCE_ENTRY MessageEntry;

        LookupSystemMessage(ERROR_BAD_EXE_FORMAT, LANG_USER_DEFAULT, &MessageEntry);

        // Convert error message to UTF-8
        if (*GetMessageEntryText(MessageEntry) == '%') {
            pos = GetMessageEntryText(MessageEntry);
            p = buffer;
        } else {
            pos = wmemchr(GetMessageEntryText(MessageEntry), '%', GetMessageEntryLength(MessageEntry));
            RtlUnicodeToUTF8N(buffer, BUFLEN, &ActualByteCount,
                GetMessageEntryText(MessageEntry), pos - GetMessageEntryText(MessageEntry));
            p = buffer + ActualByteCount;
        }

        // Convert filename to UTF-8
        RtlUnicodeToUTF8N(p, buffer + BUFLEN - p, &ActualByteCount,
            pNext, (ptr - pNext) << 1);
        p += ActualByteCount;

        pos = wmemchr(pos + 1, '1', GetMessageEntryLength(MessageEntry)) + 1;
        // Convert error message to UTF-8
        RtlUnicodeToUTF8N(p, buffer + BUFLEN - p, &ActualByteCount,
            pos, GetMessageEntryLength(MessageEntry) - ((pos - GetMessageEntryText(MessageEntry)) << 1));
        p += ActualByteCount;

        WriteDataA(hStdout, buffer, p - buffer, bConsole);
        return ERROR_BAD_EXE_FORMAT;
    }

    if (pCmdLine + len != ptr) *ptr = ' ';

    STARTUPINFOW startupInfo;
    PROCESS_INFORMATION processInfo;
    PUNICODE_STRING DosPath = GetCurrentDirectoryDosPath();

    startupInfo.cb = sizeof(startupInfo);
    startupInfo.lpReserved = NULL;
    startupInfo.lpDesktop = NULL;
    startupInfo.lpTitle = NULL;
    startupInfo.dwFlags = 0;
    startupInfo.cbReserved2 = 0;
    startupInfo.lpReserved2 = NULL;

    CreateProcessW(ApplicationName, pNext, NULL, NULL, FALSE,
        start ? CREATIONFLAGS | DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE
              : CREATIONFLAGS | DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_PROCESS_GROUP,
        GetEnvironment(), DosPath->Buffer, &startupInfo, &processInfo);

    HANDLE hProcess;
    HANDLE hFile[MAX_DLL];
    HANDLE hThread[MAX_THREAD];
    PVOID BaseOfDll[MAX_DLL] = {};
    DBGUI_WAIT_STATE_CHANGE StateChange;
    DbgWaitStateChange(&StateChange, FALSE, NULL);

    CloseHandle(processInfo.hThread);
    hFile[0] = StateChange.StateInfo.CreateProcessInfo.NewProcess.FileHandle;
    BaseOfDll[0] = StateChange.StateInfo.CreateProcessInfo.NewProcess.BaseOfImage;
    hThread[0] = StateChange.StateInfo.CreateProcessInfo.HandleToThread;
    hProcess = processInfo.hProcess;

    if (verbose >= 2) DbgTraceEvent(&StateChange, CREATE_PROCESS, strlen(CREATE_PROCESS), hStdout, bConsole);

    DbgContinue(&StateChange, DBG_CONTINUE);

    // x86 process adds one breakpoint
    if (!b64bit) --firstbreak;

    DWORD i;
    DWORD dwThreadId[MAX_THREAD] = {};

    dwThreadId[0] = HandleToUlong(StateChange.AppClientId.UniqueThread);

    while (TRUE) {
        DbgWaitStateChange(&StateChange, FALSE, NULL);

        switch (StateChange.NewState) {
        case DbgLoadDllStateChange:
            if (verbose >= 2) DbgTraceModule(StateChange.StateInfo.LoadDll.FileHandle, LOAD_DLL, strlen(LOAD_DLL), hStdout, bConsole);

            // Find storage position
            for (i = 0; i < MAX_DLL; ++i) if (!BaseOfDll[i]) {
                hFile[i] = StateChange.StateInfo.LoadDll.FileHandle;
                BaseOfDll[i] = StateChange.StateInfo.LoadDll.BaseOfDll;
                break;
            }

            break;

        case DbgUnloadDllStateChange:
            // Find specific DLL
            for (i = 0; i < MAX_DLL; ++i) if (StateChange.StateInfo.UnloadDll.BaseAddress == BaseOfDll[i]) {
                if (verbose >= 2) DbgTraceModule(hFile[i], UNLOAD_DLL, strlen(UNLOAD_DLL), hStdout, bConsole);

                CloseHandle(hFile[i]);
                BaseOfDll[i] = 0;
                break;
            }

            break;

        case DbgCreateThreadStateChange:
            if (verbose >= 2) DbgTraceEvent(&StateChange, CREATE_THREAD, strlen(CREATE_THREAD), hStdout, bConsole);

            // Find storage position
            for (i = 0; i < MAX_THREAD; ++i) if (!dwThreadId[i]) {
                hThread[i] = StateChange.StateInfo.CreateThread.HandleToThread;
                dwThreadId[i] = HandleToUlong(StateChange.AppClientId.UniqueThread);
                break;
            }

            break;

        case DbgExitThreadStateChange:
            if (verbose >= 2) DbgTraceEvent(&StateChange, EXIT_THREAD, strlen(EXIT_THREAD), hStdout, bConsole);

            // Find specific thread
            for (i = 0; i < MAX_THREAD; ++i) if (dwThreadId[i] == HandleToUlong(StateChange.AppClientId.UniqueThread)) {
                dwThreadId[i] = 0;
                break;
            }

            break;

        case DbgExitProcessStateChange:
            if (verbose >= 2) DbgTraceEvent(&StateChange, EXIT_PROCESS, strlen(EXIT_PROCESS), hStdout, bConsole);

            if (timeout)
                WaitForInputOrTimeout(hStdout, timeout, bConsole);

            CloseHandle(hProcess);

            for (i = 0; i < MAX_DLL; ++i)
                if (BaseOfDll[i]) CloseHandle(hFile[i]);

            DbgContinue(&StateChange, DBG_CONTINUE);
            return EXIT_SUCCESS;

        case DbgExceptionStateChange:
        case DbgBreakpointStateChange:
        case DbgSingleStepStateChange:
            if (StateChange.StateInfo.Exception.ExceptionRecord.ExceptionCode == DBG_PRINTEXCEPTION_WIDE_C ||
                StateChange.StateInfo.Exception.ExceptionRecord.ExceptionCode == DBG_PRINTEXCEPTION_C) {
                if (verbose >= 2) DbgTraceEvent(&StateChange, OUTPUT_DEBUG, strlen(OUTPUT_DEBUG), hStdout, bConsole);
                if (output == TRUE) DbgProcessDebugString(&StateChange, hProcess, hStdout, bConsole);
                break;
            } else if (StateChange.StateInfo.Exception.ExceptionRecord.ExceptionCode == DBG_RIPEXCEPTION) {
                if (verbose >= 2) DbgTraceEvent(&StateChange, RIP, strlen(RIP), hStdout, bConsole);
                DbgProcessRIP(&StateChange, hStdout, bConsole);
                break;
            }

            // Ignore thread naming exception
            // https://learn.microsoft.com/en-us/visualstudio/debugger/tips-for-debugging-threads
            if (StateChange.StateInfo.Exception.ExceptionRecord.ExceptionCode == MS_VC_EXCEPTION)
                break;

            // Ignore first-chance breakpoints
            if ((StateChange.StateInfo.Exception.ExceptionRecord.ExceptionCode == STATUS_BREAKPOINT ||
                StateChange.StateInfo.Exception.ExceptionRecord.ExceptionCode == STATUS_WX86_BREAKPOINT) &&
                ((breakpoint == FALSE) || (breakpoint == TRUE && ++firstbreak <= 1)))
                break;

            // Ignore other first change exceptions
            if (StateChange.StateInfo.Exception.FirstChance) {
                DbgContinue(&StateChange, DBG_EXCEPTION_NOT_HANDLED);
                continue;
            }

            // Find thread where exception occured
            for (i = 0; i < MAX_THREAD; ++i) if (HandleToUlong(StateChange.AppClientId.UniqueThread) == dwThreadId[i])
                break;

            memcpy(buffer, THREAD_NUMBER, strlen(THREAD_NUMBER));
            if (HandleToUlong(StateChange.AppClientId.UniqueThread) == dwThreadId[i]) {
                buffer[strlen(THREAD_NUMBER)] = '0' + (i + 1) / 10;
                buffer[strlen(THREAD_NUMBER) + 1] = '0' + (i + 1) % 10;
                p = buffer + strlen(THREAD_NUMBER) + 2;
            } else p = int_to_chars(buffer + strlen(THREAD_NUMBER) - 1, HandleToUlong(StateChange.AppClientId.UniqueThread));

            wchar_t Tmp[WBUFLEN];

            memcpy(p, THREAD_TRIGGERD, strlen(THREAD_TRIGGERD));
            p += strlen(THREAD_TRIGGERD);
            p = _ulto16au(StateChange.StateInfo.Exception.ExceptionRecord.ExceptionCode, p);
            *p++ = '\n';

            PMESSAGE_RESOURCE_ENTRY MessageEntry;

            p += DbgFormatException(StateChange.StateInfo.Exception.ExceptionRecord.ExceptionCode,
                LANG_USER_DEFAULT, p, buffer + BUFLEN - p, bConsole);

            if (HandleToUlong(StateChange.AppClientId.UniqueThread) != dwThreadId[i]) {
                WriteDataA(hStdout, buffer, p - buffer, bConsole);
                continue;
            }

            SymSetOptions(SYMOPTIONS);
            SymInitializeW(hProcess, NULL, FALSE);

            for (DWORD j = 0; j < MAX_DLL; ++j) if (BaseOfDll[j]) {
                DWORD DllSize;

                GetModuleSize(hFile[j], &DllSize);
                SymLoadModuleExW(hProcess, hFile[j], NULL, NULL,
                    (DWORD64) BaseOfDll[j], DllSize, NULL, 0);
            }

            CONTEXT Context;
            DWORD MachineType;
            STACKFRAME_EX StackFrame;

            if (b64bit) {
                MachineType = IMAGE_FILE_MACHINE_AMD64;
                Context.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
                NtGetContextThread(hThread[i], &Context);
                StackFrame.AddrPC.Offset = Context.Rip;
                StackFrame.AddrFrame.Offset = Context.Rbp;
                StackFrame.AddrStack.Offset = Context.Rsp;
            } else {
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
            UserContext.bConsole = bConsole;
            UserContext.pBase = &StackFrame.AddrFrame.Offset;
            UserContext.pContext = &Context;
            UserContext.hProcess = hProcess;
            UserContext.b64bit = b64bit;
            UserContext.pEnd = buffer + BUFLEN;
            DirLen = DosPath->Length;

            while (TRUE) {
                StackFrame.InlineFrameContext = INLINE_FRAME_CONTEXT_IGNORE;

                if (!StackWalk2(MachineType, hProcess, hThread[i], &StackFrame, &Context,
                    NULL, NULL, NULL, NULL, NULL, SYM_STKWALK_DEFAULT)) break;

                *p++ = '#';

                if (count < 10) {
                    *p++ = '0' + count;
                    *p = ' ';
                } else if (count < 100) {
                    *p++ = '0' + count / 10;
                    *p = '0' + count % 10;
                } else break;

                ++p;
                *p++ = ' ';

                if (bConsole) {
                    memcpy(p, CONSOLE_BLUE_FORMAT, strlen(CONSOLE_BLUE_FORMAT));
                    p += strlen(CONSOLE_BLUE_FORMAT);
                }

                p = __ui64toaddr(StackFrame.AddrPC.Offset, p, b64bit);

                if (bConsole) {
                    memcpy(p, EXCEPTION_IN, strlen(EXCEPTION_IN));
                    p += strlen(EXCEPTION_IN);
                } else {
                    memcpy(p, EXCEPTION_IN + 3, 4);
                    p += 4;
                }

                //
                //  StackFrame.AddrPC.Offset stores the return address rather than the caller's address.
                //  Adjust StackFrame.AddrPC.Offset by 'count' to resolve the caller's address accurately.
                //

                if (SymFromInlineContextW(hProcess, StackFrame.AddrPC.Offset - count,
                    INLINE_FRAME_CONTEXT_IGNORE, NULL, pSymInfo)) {
                    RtlUnicodeToUTF8N(p, buffer + BUFLEN - p,
                        &ActualByteCount, pSymInfo->Name, pSymInfo->NameLen << 1);
                    p += ActualByteCount;
                } else {
                    *p++ = '?';
                    *p++ = '?';
                }

                if (bConsole) {
                    memcpy(p, CONSOLE_DEFAULT_FORMAT,
                        strlen(CONSOLE_DEFAULT_FORMAT));
                    p += strlen(CONSOLE_DEFAULT_FORMAT);
                }

                *p++ = ' ';
                *p++ = '(';

                if (SymSetContext(hProcess, (PIMAGEHLP_STACK_FRAME) &StackFrame, &Context)) {
                    Success = TRUE;
                    UserContext.p = p;
                    UserContext.DataIsParam = TRUE;
                    SymEnumSymbolsExW(hProcess, 0, NULL, EnumSymbolsProcW, &UserContext, SYMENUM_OPTIONS_DEFAULT);
                    if (p != UserContext.p) p = UserContext.p - 2;
                } else Success = FALSE;

                *p++ = ')';
                *p++ = ' ';

                // Without &Displacement => undefined behavior
                if (SymGetLineFromInlineContextW(hProcess, StackFrame.AddrPC.Offset - count,
                    INLINE_FRAME_CONTEXT_IGNORE, NULL, &Displacement, &Line)) {
                    memcpy(p, EXCEPTION_AT, strlen(EXCEPTION_AT));
                    temp = wcslen(Line.FileName) << 1;
                    p += strlen(EXCEPTION_AT);

                    if (bConsole) {
                        memcpy(p, CONSOLE_GREEN_FORMAT, strlen(CONSOLE_GREEN_FORMAT));
                        p += strlen(CONSOLE_GREEN_FORMAT);
                    }

                    // Skip %dir%/
                    if (temp > DirLen && !memcmp(Line.FileName,
                            DosPath->Buffer, DirLen))
                        p = FormatFileLine(Line.FileName + (DirLen >> 1),
                            Line.LineNumber, temp - DirLen, buffer + BUFLEN - p, p, bConsole);
                    else p = FormatFileLine(Line.FileName,
                        Line.LineNumber, temp, buffer + BUFLEN - p, p, bConsole);
                    if (verbose >= 1) p = FormatSourceCode(Line.FileName,
                        Line.LineNumber, temp, buffer + BUFLEN - p, p);
                } else {
                    memcpy(p, EXCEPTION_FROM, strlen(EXCEPTION_FROM));
                    p += strlen(EXCEPTION_FROM);

                    if (bConsole) {
                        memcpy(p, CONSOLE_GREEN_FORMAT, strlen(CONSOLE_GREEN_FORMAT));
                        p += strlen(CONSOLE_GREEN_FORMAT);
                    }

                    len = GetModuleFileNameExW(hProcess,
                        (HMODULE) SymGetModuleBase64(hProcess, StackFrame.AddrPC.Offset), Tmp, WBUFLEN);
                    RtlUnicodeToUTF8N(p, buffer + BUFLEN - p,
                        &ActualByteCount, Tmp, len << 1);
                    p += ActualByteCount;

                    if (bConsole) {
                        memcpy(p, CONSOLE_DEFAULT_FORMAT,
                            strlen(CONSOLE_DEFAULT_FORMAT));
                        p += strlen(CONSOLE_DEFAULT_FORMAT);
                    }

                    *p++ = '\n';
                }

                if (Success && verbose >= 1) {
                    UserContext.p = p;
                    UserContext.DataIsParam = FALSE;
                    SymEnumSymbolsExW(hProcess, 0, NULL, EnumSymbolsProcW, &UserContext, SYMENUM_OPTIONS_DEFAULT);
                    p = UserContext.p;
                }

                // Release buffer
                if (p >= buffer + (sizeof(buffer) >> 1)) {
                    WriteDataA(hStdout, buffer, p - buffer, bConsole);
                    p = buffer;
                }

                ++count;
            }

            WriteDataA(hStdout, buffer, p - buffer, bConsole);

            SymCleanup(hProcess);
            NtTerminateProcess(hProcess,
                RtlNtStatusToDosError(StateChange.StateInfo.Exception.ExceptionRecord.ExceptionCode));
            break;
        }

        DbgContinue(&StateChange, DBG_CONTINUE);
    }
}