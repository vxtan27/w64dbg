// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

// Config
#include <config/crt.h>
#include <config/build.h>

#include <ntdll.h>
#include <dbghelp.h>
#include <psapi.h>

#include <conversion/status.h>
#include <conversion/address.h>
#include <conversion/decimal.h>

#include <config/core.h>
#include <exception.h>
#include <debugger/core.cpp>
#include <utils.h>
#include <fmt.h>
#include <log.h>
#include <symbols.h>

int
WINAPI
wmain(void) {
    BOOL firstbreak = DEFAULT_FIRSTBREAK,
    breakpoint = DEFAULT_BREAKPOINT,
    verbose = DEFAULT_VERBOSE,
    output = DEFAULT_OUTPUT,
    pause = DEFAULT_PAUSE;

    BOOL bConsole;
    HANDLE hStdout = RtlStandardOutput();
    PUNICODE_STRING pCommandLine = RtlCommandLine();
    size_t len = pCommandLine->Length >> 1;
    wchar_t *pCmdLine = wmemchr(pCommandLine->Buffer, ' ', len);
    wchar_t *pNext = pCmdLine;

    IsConsoleHandle(hStdout, &bConsole);

    if (pCmdLine) {
        len -= pCmdLine - pCommandLine->Buffer;
        // Modified for processing command-line arguments
        *(pCmdLine + len) = ' ';

        while (TRUE) {
            while (*pNext == ' ') ++pNext; // Skip spaces

            if (*pNext != '/' || pCmdLine + len < pNext) break;

            if (*(pNext + 2) == ' ') switch (*(pNext + 1)) {
            case 'B':
                breakpoint = FALSE;
                pNext += 3;
                continue;

            case 'O':
                output = FALSE;
                pNext += 3;
                continue;

            case 'T':
                pause = TRUE;
                pNext += 3;
                continue;

            case 'V':
                verbose = 2;
                pNext += 3;
                continue;

            case '?':
                WriteHandle(hStdout, (PVOID) (HELP + 16),
                    strlen(HELP) - 16, FALSE, bConsole);
                return EXIT_SUCCESS;
            }

            if (*(pNext + 1) == 'V' && *(pNext + 2) >= '0' &&
                    *(pNext + 2) <= '2' && *(pNext + 3) == ' ') {
                verbose = *(pNext + 2) - '0';
                pNext += 4;
                continue;
            }

            WriteInvalidArgument(hStdout, pNext, wmemchr(pNext,
                ' ', pCmdLine + len + 1 - pNext) - pNext, bConsole);
            return ERROR_INVALID_PARAMETER;
        }
    }

    // No executable specified
    if (!pCmdLine || pCmdLine + len < pNext) {
        WriteHandle(hStdout, (PVOID) HELP, 65, FALSE, bConsole);
        return ERROR_BAD_ARGUMENTS;
    }

    wchar_t ApplicationName[WBUFLEN];

    wchar_t *ptr = wmemchr(pNext, ' ',
        pCmdLine + len + 1 - pNext);
    *ptr = '\0';
    *(pCmdLine + len) = '\0';

    if (!SearchPathW(NULL, pNext, EXTENSION,
        sizeof(ApplicationName) >> 1, ApplicationName, NULL)) { // Check if executable exists
        PMESSAGE_RESOURCE_ENTRY MessageEntry;

        LookupSystemMessage(ERROR_FILE_NOT_FOUND, LANG_USER_DEFAULT, &MessageEntry);
        WriteHandle(hStdout, GetMessageEntryText(MessageEntry),
            GetMessageEntryLength(MessageEntry), TRUE, bConsole);
        return ERROR_FILE_NOT_FOUND;
    }

    char *p;
    DWORD b64bit; // Is 64-bit application
    char buffer[BUFLEN];

    // Check if executable format (x86-64)
    if (!GetBinaryTypeW(ApplicationName, &b64bit) ||
        (b64bit != SCS_32BIT_BINARY && b64bit != SCS_64BIT_BINARY)) {
        ULONG ActualByteCount;
        PMESSAGE_RESOURCE_ENTRY MessageEntry;

        LookupSystemMessage(ERROR_BAD_EXE_FORMAT, LANG_USER_DEFAULT, &MessageEntry);

        PWCH pos;
        PCWCH Text = (PCWCH) GetMessageEntryText(MessageEntry);
        // Convert error message to UTF-8
        if (*Text == '%') {
            pos = (PWCH) Text;
            p = buffer;
        } else {
            pos = (PWCH) wmemchr(Text, '%', GetMessageEntryLength(MessageEntry));
            RtlUnicodeToUTF8N(buffer, BUFLEN, &ActualByteCount,
                Text, pos - Text);
            p = buffer + ActualByteCount;
        }

        // Convert filename to UTF-8
        RtlUnicodeToUTF8N(p, buffer + BUFLEN - p, &ActualByteCount,
            pNext, (ptr - pNext) << 1);
        p += ActualByteCount;

        pos = wmemchr(pos + 1, '1', GetMessageEntryLength(MessageEntry)) + 1;
        // Convert error message to UTF-8
        RtlUnicodeToUTF8N(p, buffer + BUFLEN - p, &ActualByteCount,
            pos, GetMessageEntryLength(MessageEntry) - ((pos - Text) << 1));
        p += ActualByteCount;

        WriteHandle(hStdout, buffer, p - buffer, FALSE, bConsole);
        return ERROR_BAD_EXE_FORMAT;
    }

    if (pCmdLine + len != ptr) *ptr = ' ';

    STARTUPINFOW startupInfo;
    PROCESS_INFORMATION processInfo;
    PUNICODE_STRING DosPath = RtlDosPath();

    startupInfo.cb = sizeof(startupInfo);
    startupInfo.lpReserved = NULL;
    startupInfo.lpDesktop = NULL;
    startupInfo.lpTitle = NULL;
    startupInfo.dwFlags = 0;
    startupInfo.cbReserved2 = 0;
    startupInfo.lpReserved2 = NULL;

    CreateProcessW(ApplicationName, pNext, NULL, NULL, FALSE,
        CREATIONFLAGS | DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_PROCESS_GROUP,
        RtlEnvironment(), DosPath->Buffer, &startupInfo, &processInfo);

    HANDLE hProcess;
    HANDLE hFile[MAX_DLL];
    PVOID BaseOfDll[MAX_DLL] = {};
    DBGUI_WAIT_STATE_CHANGE StateChange;
    DbgWaitStateChange(&StateChange, FALSE, NULL);

    NtClose(processInfo.hThread);
    hFile[0] = StateChange.StateInfo.CreateProcessInfo.NewProcess.FileHandle;
    BaseOfDll[0] = StateChange.StateInfo.CreateProcessInfo.NewProcess.BaseOfImage;
    hProcess = processInfo.hProcess;

    if (verbose >= 2) TraceDebugEvent(&StateChange, CREATE_PROCESS, strlen(CREATE_PROCESS), hStdout, bConsole);

    DbgContinue(&StateChange, DBG_CONTINUE);

    // x86 process adds one breakpoint
    if (!b64bit) --firstbreak;

    DWORD i;

    while (NT_SUCCESS(DbgWaitStateChange(&StateChange, FALSE, NULL))) {
        switch (StateChange.NewState) {
        case DbgLoadDllStateChange:
            if (verbose >= 2) TraceDebugModule(StateChange.StateInfo.LoadDll.FileHandle, LOAD_DLL, strlen(LOAD_DLL), hStdout, bConsole);

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
                if (verbose >= 2) TraceDebugModule(hFile[i], UNLOAD_DLL, strlen(UNLOAD_DLL), hStdout, bConsole);

                NtClose(hFile[i]);
                BaseOfDll[i] = 0;
                break;
            }

            break;

        case DbgCreateThreadStateChange:
            if (verbose >= 2) TraceDebugEvent(&StateChange, CREATE_THREAD, strlen(CREATE_THREAD), hStdout, bConsole);
            break;

        case DbgExitThreadStateChange:
            if (verbose >= 2) TraceDebugEvent(&StateChange, EXIT_THREAD, strlen(EXIT_THREAD), hStdout, bConsole);
            break;

        case DbgExitProcessStateChange:
            if (verbose >= 2) TraceDebugEvent(&StateChange, EXIT_PROCESS, strlen(EXIT_PROCESS), hStdout, bConsole);

            if (pause) WaitForKeyPress(hStdout, bConsole);

            NtClose(hProcess);

            for (i = 0; i < MAX_DLL; ++i)
                if (BaseOfDll[i]) NtClose(hFile[i]);

            DbgContinue(&StateChange, DBG_CONTINUE);
            return EXIT_SUCCESS;

        case DbgExceptionStateChange:
        case DbgBreakpointStateChange:
        case DbgSingleStepStateChange:
            if (StateChange.StateInfo.Exception.ExceptionRecord.ExceptionCode == DBG_PRINTEXCEPTION_WIDE_C ||
                StateChange.StateInfo.Exception.ExceptionRecord.ExceptionCode == DBG_PRINTEXCEPTION_C) {
                if (verbose >= 2) TraceDebugEvent(&StateChange, OUTPUT_DEBUG, strlen(OUTPUT_DEBUG), hStdout, bConsole);
                if (output == TRUE) ProcessDebugStringEvent(&StateChange, hProcess, hStdout, bConsole);
                break;
            } else if (StateChange.StateInfo.Exception.ExceptionRecord.ExceptionCode == DBG_RIPEXCEPTION) {
                if (verbose >= 2) TraceDebugEvent(&StateChange, RIP, strlen(RIP), hStdout, bConsole);
                ProcessRIPEvent(&StateChange, hStdout, bConsole);
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

            memcpy(buffer, THREAD_NUMBER, strlen(THREAD_NUMBER));
            p = conversion::dec::from_int(buffer + strlen(THREAD_NUMBER),
                HandleToUlong(StateChange.AppClientId.UniqueThread));
            *p = 'x';
            p = conversion::dec::from_int(p + 1,
                HandleToUlong(StateChange.AppClientId.UniqueProcess));

            memcpy(p, THREAD_RAISED, strlen(THREAD_RAISED));
            p += strlen(THREAD_RAISED);
            p = conversion::status::from_int(p,
                StateChange.StateInfo.Exception.ExceptionRecord.ExceptionCode);
            *p++ = '\n';

            p += FormatExceptionEvent(StateChange.StateInfo.Exception.ExceptionRecord.ExceptionCode,
                LANG_USER_DEFAULT, p, buffer + BUFLEN - p, bConsole);

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
            HANDLE hThread = DbgGetThreadHandle(&StateChange);

            if (b64bit) {
                MachineType = IMAGE_FILE_MACHINE_AMD64;
                Context.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
                NtGetContextThread(hThread, &Context);
                StackFrame.AddrPC.Offset = Context.Rip;
                StackFrame.AddrFrame.Offset = Context.Rbp;
                StackFrame.AddrStack.Offset = Context.Rsp;
            } else {
                MachineType = IMAGE_FILE_MACHINE_I386;
                ((PWOW64_CONTEXT) &Context)->ContextFlags = WOW64_CONTEXT_CONTROL | WOW64_CONTEXT_INTEGER;
                NtQueryInformationThread(hThread, ThreadWow64Context,
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

            if (bConsole) {
                SetConsoleDeviceOutputCP(hStdout, 65001);  // CP_UTF8
                SetConsoleDeviceMode(hStdout, ENABLE_PROCESSED_OUTPUT |
                    ENABLE_WRAP_AT_EOL_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
            }

            while (TRUE) {
                StackFrame.InlineFrameContext = INLINE_FRAME_CONTEXT_IGNORE;

                if (!StackWalk2(MachineType, hProcess, hThread, &StackFrame, &Context,
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

                if (b64bit)
                    p = conversion::addr::from_int(p, StackFrame.AddrPC.Offset);
                else p = conversion::addr::from_int(p, (DWORD) StackFrame.AddrPC.Offset);

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
                    p += ConvertUnicodeToUTF8(pSymInfo->Name,
                        pSymInfo->NameLen << 1, p, buffer + BUFLEN - p);
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
                    size_t temp = wcslen(Line.FileName) << 1;
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

                    wchar_t Tmp[WBUFLEN];
                    len = GetModuleFileNameExW(hProcess,
                        (HMODULE) SymGetModuleBase64(hProcess, StackFrame.AddrPC.Offset), Tmp, WBUFLEN);
                    p += ConvertUnicodeToUTF8(Tmp, len << 1, p, buffer + BUFLEN - p);

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
                    WriteHandle(hStdout, buffer, p - buffer, FALSE, bConsole);
                    p = buffer;
                }

                ++count;
            }

            WriteHandle(hStdout, buffer, p - buffer, FALSE, bConsole);

            SymCleanup(hProcess);
            NtTerminateProcess(hProcess,
                RtlNtStatusToDosError(StateChange.StateInfo.Exception.ExceptionRecord.ExceptionCode));
            break;
        }

        DbgContinue(&StateChange, DBG_CONTINUE);
    }
}