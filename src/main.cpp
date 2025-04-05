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

#include <debugger/core.cpp>
#include <config/core.h>
#include <exception.h>
#include <utils.h>
#include <fmt.h>
#include <log.h>
#include <symbols.h>

int
WINAPI
wmain(void) {
    BOOL fVerbose = DEFAULT_VERBOSE,
    fPauseExecution = DEFAULT_PAUSE,
    fOutputDebugString = DEFAULT_OUTPUT,
    fIgnoreBreakpoints = DEFAULT_BREAKPOINT;

    BOOL bConsole;
    HANDLE hStdout = RtlStandardOutput();
    PUNICODE_STRING pCommandLine = RtlCommandLine();
    size_t len = pCommandLine->Length >> 1;
    wchar_t *pCmdLine = wmemchr(pCommandLine->Buffer, ' ', len);
    wchar_t *pNext = pCmdLine + 1;

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
                fIgnoreBreakpoints = FALSE;
                pNext += 3;
                continue;

            case 'O':
                fOutputDebugString = FALSE;
                pNext += 3;
                continue;

            case 'T':
                fPauseExecution = TRUE;
                pNext += 3;
                continue;

            case 'V':
                fVerbose = 1;
                pNext += 3;
                continue;

            case '?':
                WriteHandle(hStdout, (PVOID) (HELP + 23),
                    strlen(HELP) - 23, FALSE, bConsole);
                return EXIT_SUCCESS;
            }

            if (*(pNext + 1) == 'V' && *(pNext + 2) >= '0' &&
                    *(pNext + 2) <= '2' && *(pNext + 3) == ' ') {
                fVerbose = *(pNext + 2) - '0';
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
        WriteHandle(hStdout, (PVOID) HELP, 72, FALSE, bConsole);
        return ERROR_BAD_ARGUMENTS;
    }

    wchar_t *ptr = wmemchr(pNext, ' ',
        pCmdLine + len + 1 - pNext);
    *ptr = '\0';
    *(pCmdLine + len) = '\0';

    UNICODE_STRING NtPathName;

    // Get the NT Path
    RtlDosPathNameToNtPathName_U_WithStatus(pNext,
                                            &NtPathName,
                                            NULL,
                                            NULL);

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

    if (!CreateProcessW(NtPathName.Buffer, pNext, NULL,
        NULL, FALSE, CREATIONFLAGS, RtlEnvironment(),
        DosPath->Buffer, &startupInfo, &processInfo)) {
        PMESSAGE_RESOURCE_ENTRY MessageEntry;
        LookupSystemMessage(GetLastError(), LANG_USER_DEFAULT, &MessageEntry);
        WriteHandle(hStdout, GetMessageEntryText(MessageEntry),
            GetMessageEntryLength(MessageEntry), TRUE, bConsole);
        return GetLastError();
    }

    // Free the buffer
    RtlFreeHeap(RtlProcessHeap(), HEAP_NO_SERIALIZE, NtPathName.Buffer);

    HANDLE hProcess;
    HANDLE hFile[MAX_DLL];
    PVOID BaseOfDll[MAX_DLL] = {};
    DBGUI_WAIT_STATE_CHANGE StateChange;
    DbgWaitStateChange(&StateChange, FALSE, NULL);

    NtClose(processInfo.hThread);
    hFile[0] = StateChange.StateInfo.CreateProcessInfo.NewProcess.FileHandle;
    BaseOfDll[0] = StateChange.StateInfo.CreateProcessInfo.NewProcess.BaseOfImage;
    hProcess = processInfo.hProcess;

    if (fVerbose >= 2) TraceDebugEvent(&StateChange, CREATE_PROCESS, strlen(CREATE_PROCESS), hStdout, bConsole);

    DbgContinue(&StateChange, DBG_CONTINUE);

    DWORD b64bit; // Is 64-bit application
    BOOL fBreakpointSignalled = FALSE;
    SYSTEM_SUPPORTED_PROCESSOR_ARCHITECTURES_INFORMATION Machines[4];

    NtQuerySystemInformationEx(SystemSupportedProcessorArchitectures2,
        &hProcess, sizeof(hProcess), Machines, sizeof(Machines), NULL);

    for (int i = 0; Machines[i].Machine; ++i) {
        b64bit = Machines[i].Process;
        break;
    }

    if (!b64bit) --fBreakpointSignalled; // Wow64 breakpoint

    NTSTATUS NtStatus;
    while (NT_SUCCESS(NtStatus = DbgWaitStateChange(&StateChange, FALSE, NULL))) {
        switch (StateChange.NewState) {
        case DbgLoadDllStateChange:
            if (fVerbose >= 2) TraceDebugModule(StateChange.StateInfo.LoadDll.FileHandle, LOAD_DLL, strlen(LOAD_DLL), hStdout, bConsole);

            // Find storage position
            for (int i = 0; i < MAX_DLL; ++i) if (!BaseOfDll[i]) {
                hFile[i] = StateChange.StateInfo.LoadDll.FileHandle;
                BaseOfDll[i] = StateChange.StateInfo.LoadDll.BaseOfDll;
                break;
            }

            break;

        case DbgUnloadDllStateChange:
            // Find specific DLL
            for (int i = 0; i < MAX_DLL; ++i) if (StateChange.StateInfo.UnloadDll.BaseAddress == BaseOfDll[i]) {
                if (fVerbose >= 2) TraceDebugModule(hFile[i], UNLOAD_DLL, strlen(UNLOAD_DLL), hStdout, bConsole);

                NtClose(hFile[i]);
                BaseOfDll[i] = 0;
                break;
            }

            break;

        case DbgCreateThreadStateChange:
            if (fVerbose >= 2) TraceDebugEvent(&StateChange, CREATE_THREAD, strlen(CREATE_THREAD), hStdout, bConsole);
            break;

        case DbgExitThreadStateChange:
            if (fVerbose >= 2) TraceDebugEvent(&StateChange, EXIT_THREAD, strlen(EXIT_THREAD), hStdout, bConsole);
            break;

        case DbgExitProcessStateChange:
            if (fVerbose >= 2) TraceDebugEvent(&StateChange, EXIT_PROCESS, strlen(EXIT_PROCESS), hStdout, bConsole);

            if (fPauseExecution) WaitForKeyPress(hStdout, bConsole);

            NtClose(hProcess);

            for (int i = 0; i < MAX_DLL; ++i)
                if (BaseOfDll[i]) NtClose(hFile[i]);

            DbgContinue(&StateChange, DBG_CONTINUE);
            return EXIT_SUCCESS;

        case DbgExceptionStateChange:
        case DbgBreakpointStateChange:
        case DbgSingleStepStateChange:
            PDBGKM_EXCEPTION pException = &StateChange.StateInfo.Exception;
            PEXCEPTION_RECORD pExceptionRecord = &pException->ExceptionRecord;
            if (pExceptionRecord->ExceptionCode == DBG_PRINTEXCEPTION_WIDE_C ||
                pExceptionRecord->ExceptionCode == DBG_PRINTEXCEPTION_C) {
                if (fVerbose >= 2) TraceDebugEvent(&StateChange, OUTPUT_DEBUG, strlen(OUTPUT_DEBUG), hStdout, bConsole);
                if (fOutputDebugString == TRUE) ProcessOutputDebugStringEvent(&StateChange, hProcess, hStdout, bConsole);
                break;
            } else if (pExceptionRecord->ExceptionCode == DBG_RIPEXCEPTION) {
                if (fVerbose >= 2) TraceDebugEvent(&StateChange, RIP, strlen(RIP), hStdout, bConsole);
                ProcessRIPEvent(&StateChange, hStdout, bConsole);
                break;
            }

            // Ignore thread naming exception
            // https://learn.microsoft.com/en-us/visualstudio/debugger/tips-for-debugging-threads
            if (pExceptionRecord->ExceptionCode == MS_VC_EXCEPTION)
                break;

            // Ignore signal breakpoints
            if ((pExceptionRecord->ExceptionCode == STATUS_BREAKPOINT ||
                pExceptionRecord->ExceptionCode == STATUS_WX86_BREAKPOINT) &&
                ((fIgnoreBreakpoints == FALSE) || (fIgnoreBreakpoints == TRUE && ++fBreakpointSignalled <= 1)))
                break;

            // Ignore other first change exceptions
            if (pException->FirstChance) {
                DbgContinue(&StateChange, DBG_EXCEPTION_NOT_HANDLED);
                continue;
            }

            char buffer[BUFLEN];
            memcpy(buffer, THREAD_NUMBER, strlen(THREAD_NUMBER));
            char *p = conversion::dec::from_int(buffer + strlen(THREAD_NUMBER),
                DbgGetProcessId(&StateChange));
            *p = 'x';
            p = conversion::dec::from_int(p + 1, DbgGetThreadId(&StateChange));

            memcpy(p, THREAD_RAISED, strlen(THREAD_RAISED));
            p += strlen(THREAD_RAISED);
            p = conversion::status::from_int(p, pExceptionRecord->ExceptionCode);
            *p++ = '\n';

            p += FormatExceptionEvent(pExceptionRecord->ExceptionCode,
                LANG_USER_DEFAULT, p, buffer + BUFLEN - p, bConsole);

            SymSetOptions(SYMOPTIONS);
            SymInitializeW(hProcess, NULL, FALSE);

            for (int i = 0; i < MAX_DLL; ++i) if (BaseOfDll[i]) {
                DWORD DllSize;

                GetModuleSize(hFile[i], &DllSize);
                SymLoadModuleExW(hProcess, hFile[i], NULL, NULL,
                    (DWORD64) BaseOfDll[i], DllSize, NULL, 0);
            }

            DWORD MachineType;
            STACKFRAME_EX StackFrame;
            BYTE Context[FIELD_OFFSET(CONTEXT, FltSave)]; // CONTEXT Context
            HANDLE hThread = DbgGetThreadHandle(&StateChange);

            if (b64bit) {
                MachineType = IMAGE_FILE_MACHINE_AMD64;
                ((PCONTEXT) &Context)->ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
                NtGetContextThread(hThread, (PCONTEXT) &Context);
                StackFrame.AddrPC.Offset = ((PCONTEXT) &Context)->Rip;
                StackFrame.AddrFrame.Offset = ((PCONTEXT) &Context)->Rbp;
                StackFrame.AddrStack.Offset = ((PCONTEXT) &Context)->Rsp;
            } else {
                MachineType = IMAGE_FILE_MACHINE_I386;
                ((PWOW64_CONTEXT) &Context)->ContextFlags = WOW64_CONTEXT_CONTROL | WOW64_CONTEXT_INTEGER;
                NtQueryInformationThread(hThread, ThreadWow64Context, &Context, sizeof(WOW64_CONTEXT), NULL);
                StackFrame.AddrPC.Offset = ((PWOW64_CONTEXT) &Context)->Eip;
                StackFrame.AddrFrame.Offset = ((PWOW64_CONTEXT) &Context)->Ebp;
                StackFrame.AddrStack.Offset = ((PWOW64_CONTEXT) &Context)->Esp;
            }

            StackFrame.AddrPC.Mode = AddrModeFlat;
            StackFrame.AddrFrame.Mode = AddrModeFlat;
            StackFrame.AddrStack.Mode = AddrModeFlat;
            // StackFrame.StackFrameSize = sizeof(StackFrame);

            DWORD count;
            BOOL Success;
            DWORD DirLen;
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
                    if (fVerbose >= 1) p = FormatSourceCode(Line.FileName,
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

                if (Success && fVerbose >= 1) {
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

            DWORD dwExitCode = RtlNtStatusToDosError(pExceptionRecord->ExceptionCode);
            NtTerminateProcess(hProcess,
                dwExitCode != ERROR_MR_MID_NOT_FOUND ? dwExitCode : pExceptionRecord->ExceptionCode);
            break;
        }

        DbgContinue(&StateChange, DBG_CONTINUE);
    }

    DWORD dwExitCode = RtlNtStatusToDosError(NtStatus);
    return dwExitCode != ERROR_MR_MID_NOT_FOUND ? dwExitCode : NtStatus;
}