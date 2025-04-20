// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

// Configuration
#include "config/crt.h"
#include "config/build.h"

// Conversion
#include "conversion/status.h"
#include "conversion/address.h"
#include "conversion/decimal.h"

#include "ntdll.h"
#include <dbghelp.h>
#include <psapi.h>

#include "debug/core.cpp"
#include "config/core.h"
#include "debugger.h"
#include "utils.h"
#include "log.h"
#include "symbols.h"

int __stdcall wmain(void) {
    BOOL fVerbose = DEFAULT_VERBOSE,
    fPauseExecution = DEFAULT_PAUSE,
    fOutputDebugString = DEFAULT_OUTPUT,
    fIgnoreBreakpoints = DEFAULT_BREAKPOINT;

    BOOL fConsole;
    HANDLE hStdout = RtlStandardOutput();
    IsConsoleHandle(hStdout, &fConsole);

    PUNICODE_STRING pCommandLine = RtlCommandLine();
    size_t len = pCommandLine->Length >> 1;
    wchar_t *pCmdLine = wmemchr(pCommandLine->Buffer, ' ', len);
    wchar_t *pNext = pCmdLine + 1;

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
                    strlen(HELP) - 23, FALSE, fConsole);
                return EXIT_SUCCESS;
            }

            if (*(pNext + 1) == 'V' && *(pNext + 2) >= '0' &&
                    *(pNext + 2) <= '2' && *(pNext + 3) == ' ') {
                fVerbose = *(pNext + 2) - '0';
                pNext += 4;
                continue;
            }

            WriteInvalidArgument(hStdout, pNext, wmemchr(pNext,
                ' ', pCmdLine + len + 1 - pNext) - pNext, fConsole);
            return ERROR_INVALID_PARAMETER;
        }
    }

    // No executable specified
    if (!pCmdLine || pCmdLine + len < pNext) {
        WriteHandle(hStdout, (PVOID) HELP, 72, FALSE, fConsole);
        return ERROR_BAD_ARGUMENTS;
    }

    wchar_t *ptr = wmemchr(pNext, ' ',
        pCmdLine + len + 1 - pNext);

    HANDLE hProcess, hThread;
    SECTION_IMAGE_INFORMATION SectionImageInfomation;
    NTSTATUS NtStatus = InitializeDebugProcess(&hProcess,
                                               &hThread,
                                                pNext,
                                                (ptr - pNext) << 1,
                                                pNext,
                                                (pCmdLine + len - pNext) << 1,
                                                &SectionImageInfomation);

    if (NtStatus != STATUS_SUCCESS) {
        PMESSAGE_RESOURCE_ENTRY MessageEntry;
        DWORD ErrorCode = RtlNtStatusToDosErrorNoTeb(NtStatus);
        LookupSystemMessage(ErrorCode, LANG_USER_DEFAULT, &MessageEntry);
        WriteHandle(hStdout, GetMessageEntryText(MessageEntry),
            GetMessageEntryLength(MessageEntry), TRUE, fConsole);
        return ErrorCode;
    }

    // Free the unused handle
    NtClose(hThread);

    *(pCmdLine + len) = ' ';

    DBGUI_WAIT_STATE_CHANGE StateChange;
    DbgWaitStateChange(&StateChange, FALSE, NULL);

    if (StateChange.NewState != DbgCreateProcessStateChange) std::unreachable();

    HANDLE hFile[MAX_DLL];
    PVOID BaseOfDll[MAX_DLL] = {};
    hFile[0] = StateChange.StateInfo.CreateProcessInfo.NewProcess.FileHandle;
    BaseOfDll[0] = StateChange.StateInfo.CreateProcessInfo.NewProcess.BaseOfImage;

    if (fVerbose >= 2) TraceDebugEvent(&StateChange, CREATE_PROCESS, strlen(CREATE_PROCESS), hStdout, fConsole);

    WdbgContinueDebugEvent(&StateChange, DBG_CONTINUE);

    // Is 64-bit application
    DWORD b64bit = SectionImageInfomation.Machine == IMAGE_FILE_MACHINE_AMD64;
    BOOL fBreakpointSignalled = FALSE;
    if (!b64bit) --fBreakpointSignalled; // Wow64 breakpoint

    while (DbgWaitStateChange(&StateChange, FALSE, NULL) == STATUS_SUCCESS) {
        switch (StateChange.NewState) {
        case DbgLoadDllStateChange:
            if (fVerbose >= 2)
                TraceDebugModule(StateChange.StateInfo.LoadDll.FileHandle,
                    LOAD_DLL, strlen(LOAD_DLL), hStdout, fConsole);

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
                if (fVerbose >= 2)
                    TraceDebugModule(hFile[i], UNLOAD_DLL, strlen(UNLOAD_DLL), hStdout, fConsole);

                NtClose(hFile[i]);
                BaseOfDll[i] = 0;
                break;
            }

            break;

        case DbgCreateThreadStateChange:
            if (fVerbose >= 2)
                TraceDebugEvent(&StateChange, CREATE_THREAD, strlen(CREATE_THREAD), hStdout, fConsole);
            break;

        case DbgExitThreadStateChange:
            if (fVerbose >= 2)
                TraceDebugEvent(&StateChange, EXIT_THREAD, strlen(EXIT_THREAD), hStdout, fConsole);
            break;

        case DbgExitProcessStateChange:
            if (fVerbose >= 2)
                TraceDebugEvent(&StateChange, EXIT_PROCESS, strlen(EXIT_PROCESS), hStdout, fConsole);

            NtClose(hProcess);
            WdbgContinueDebugEvent(&StateChange, DBG_CONTINUE);

            for (int i = 0; i < MAX_DLL; ++i)
                if (BaseOfDll[i]) NtClose(hFile[i]);

            if (fPauseExecution) WaitForKeyPress(hStdout, fConsole);

            return EXIT_SUCCESS;

        case DbgExceptionStateChange:
        case DbgBreakpointStateChange:
        case DbgSingleStepStateChange: {
            PDBGKM_EXCEPTION pException = &StateChange.StateInfo.Exception;
            PEXCEPTION_RECORD pExceptionRecord = &pException->ExceptionRecord;

            if (pExceptionRecord->ExceptionCode == DBG_PRINTEXCEPTION_WIDE_C ||
                pExceptionRecord->ExceptionCode == DBG_PRINTEXCEPTION_C) {
                if (fVerbose >= 2)
                    TraceDebugEvent(&StateChange, OUTPUT_DEBUG, strlen(OUTPUT_DEBUG), hStdout, fConsole);
                if (fOutputDebugString == TRUE) 
                    ProcessOutputDebugStringEvent(&StateChange, hProcess, hStdout, fConsole);
                break;
            } else if (pExceptionRecord->ExceptionCode == DBG_RIPEXCEPTION) {
                if (fVerbose >= 2)
                    TraceDebugEvent(&StateChange, RIP, strlen(RIP), hStdout, fConsole);
                ProcessRIPEvent(&StateChange, hStdout, fConsole);
                break;
            }

            // Ignore signal breakpoints
            if ((pExceptionRecord->ExceptionCode == STATUS_BREAKPOINT ||
                pExceptionRecord->ExceptionCode == STATUS_WX86_BREAKPOINT) &&
                ((fIgnoreBreakpoints == FALSE) ||
                    (fIgnoreBreakpoints == TRUE && ++fBreakpointSignalled <= 1)))
                break;

            // https://learn.microsoft.com/en-us/visualstudio/debugger/tips-for-debugging-threads
            if (pExceptionRecord->ExceptionCode == MS_VC_EXCEPTION)
            {
                CHAR ThreadName[MAX_THREAD_NAME_SIZE];
                THREADNAME_INFO *pInfo = (THREADNAME_INFO*) pExceptionRecord->ExceptionInformation;
                if (NtReadVirtualMemory(hProcess, (PVOID) pInfo->szName,
                    ThreadName, MAX_THREAD_NAME_SIZE, NULL) != STATUS_SUCCESS) break;
                ThreadName[MAX_THREAD_NAME_SIZE - 1] = '\0';

                THREAD_NAME_INFORMATION NameInfo;
                RtlCreateUnicodeStringFromAsciiz(&NameInfo.ThreadName, ThreadName);
                NtSetInformationThread(
                    WdbgGetThreadHandle(pInfo->dwThreadID == -1 ?
                        DbgGetThreadId(&StateChange) : pInfo->dwThreadID),
                    ThreadNameInformation, &NameInfo, sizeof(NameInfo));
                RtlFreeHeap(RtlProcessHeap(), HEAP_NO_SERIALIZE,
                    NameInfo.ThreadName.Buffer);
                break;
            }

            // Ignore other first change exceptions
            if (pException->FirstChance) {
                WdbgContinueDebugEvent(&StateChange, DBG_EXCEPTION_NOT_HANDLED);
                continue;
            }

            char buffer[BUFLEN];
            memcpy(buffer, THREAD_NUMBER, strlen(THREAD_NUMBER));
            char *p = buffer + strlen(THREAD_NUMBER);

            ULONG ReturnLength;
            HANDLE hThread = WdbgGetThreadHandle(DbgGetThreadId(&StateChange));
            NtQueryInformationThread(hThread,
                                     ThreadNameInformation,
                                     buffer + 8,
                                     sizeof(THREAD_NAME_INFORMATION),
                                     &ReturnLength);

            if (ReturnLength != sizeof(THREAD_NAME_INFORMATION)) {
                PVOID Buffer = _alloca(ReturnLength + sizeof(WCHAR));
                PTHREAD_NAME_INFORMATION Info = (PTHREAD_NAME_INFORMATION) Buffer;
                NtQueryInformationThread(hThread, ThreadNameInformation,
                    Info, ReturnLength, NULL);
                p += ConvertUnicodeToUTF8(Info->ThreadName.Buffer,
                    Info->ThreadName.Length, p, MAX_THREAD_NAME_SIZE);
            } else {
                p = conversion::dec::from_int(p, DbgGetThreadId(&StateChange));
            }

            memcpy(p, THREAD_RAISED, strlen(THREAD_RAISED));
            p += strlen(THREAD_RAISED);
            p = conversion::status::from_int(p, pExceptionRecord->ExceptionCode);
            *p++ = '\n';

            p += FormatExceptionEvent(pExceptionRecord->ExceptionCode,
                LANG_USER_DEFAULT, p, buffer + BUFLEN - p, fConsole);

            STACKFRAME_EX StackFrame;
            BYTE Context[FIELD_OFFSET(CONTEXT, FltSave)]; // CONTEXT Context

            if (b64bit) {
                ((PCONTEXT) &Context)->ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
                NtGetContextThread(hThread, (PCONTEXT) &Context);
                StackFrame.AddrPC.Offset = ((PCONTEXT) &Context)->Rip;
                StackFrame.AddrFrame.Offset = ((PCONTEXT) &Context)->Rbp;
                StackFrame.AddrStack.Offset = ((PCONTEXT) &Context)->Rsp;
            } else {
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
            IMAGEHLP_LINEW64 Line;
            USERCONTEXT UserContext;
            char Symbol[sizeof(SYMBOL_INFO) + (MAX_SYM_NAME << 1)];

            PSYMBOL_INFOW pSymInfo = (PSYMBOL_INFOW) Symbol;
            Line.SizeOfStruct = sizeof(Line);
            pSymInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
            pSymInfo->MaxNameLen = MAX_SYM_NAME;

            count = 0;
            UserContext.fConsole = fConsole;
            UserContext.pBase = &StackFrame.AddrFrame.Offset;
            UserContext.pContext = &Context;
            UserContext.hProcess = hProcess;
            UserContext.b64bit = b64bit;
            UserContext.pEnd = buffer + BUFLEN;

            if (fConsole) {
                SetConsoleDeviceOutputCP(hStdout, 65001);  // CP_UTF8
                SetConsoleDeviceMode(hStdout, ENABLE_PROCESSED_OUTPUT |
                    ENABLE_WRAP_AT_EOL_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
            }

            SymSetOptions(SYM_OPTIONS);
            SymInitializeW(hProcess, NULL, FALSE);

            for (int i = 0; i < MAX_DLL; ++i) if (BaseOfDll[i]) {
                SIZE_T DllSize;
                GetModuleSize(hProcess, BaseOfDll[i], &DllSize);
                SymLoadModuleExW(hProcess, hFile[i], NULL, NULL,
                    (DWORD64) BaseOfDll[i], DllSize, NULL, 0);
            }

            while (TRUE) {
                StackFrame.InlineFrameContext = INLINE_FRAME_CONTEXT_IGNORE;

                if (!StackWalk2(SectionImageInfomation.Machine,
                    hProcess, hThread, &StackFrame, &Context, NULL,
                    NULL, NULL, NULL, NULL, SYM_STKWALK_DEFAULT)) break;

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

                if (fConsole) {
                    memcpy(p, CONSOLE_BLUE_FORMAT, strlen(CONSOLE_BLUE_FORMAT));
                    p += strlen(CONSOLE_BLUE_FORMAT);
                }

                if (b64bit)
                    p = conversion::addr::from_int(p, StackFrame.AddrPC.Offset);
                else p = conversion::addr::from_int(p, (DWORD) StackFrame.AddrPC.Offset);

                if (fConsole) {
                    memcpy(p, EXCEPTION_IN, strlen(EXCEPTION_IN));
                    p += strlen(EXCEPTION_IN);
                } else {
                    memcpy(p, EXCEPTION_IN + 3, 4);
                    p += 4;
                }

                if (SymFromInlineContextW(hProcess, StackFrame.AddrPC.Offset,
                    INLINE_FRAME_CONTEXT_IGNORE, NULL, pSymInfo)) {
                    p += ConvertUnicodeToUTF8(pSymInfo->Name,
                        pSymInfo->NameLen << 1, p, buffer + BUFLEN - p);
                } else {
                    *p++ = '?';
                    *p++ = '?';
                }

                if (fConsole) {
                    memcpy(p, CONSOLE_DEFAULT_FORMAT,
                        strlen(CONSOLE_DEFAULT_FORMAT));
                    p += strlen(CONSOLE_DEFAULT_FORMAT);
                }

                *p++ = ' ';
                *p++ = '(';

                // IMAGEHLP_STACK_FRAME ImageHlpStackFrame = {
                //     .InstructionOffset = StackFrame.AddrPC.Offset
                // };

                // if (SymSetContext(hProcess, &ImageHlpStackFrame, NULL)) {
                if (SymSetContext(hProcess, (PIMAGEHLP_STACK_FRAME) &StackFrame, NULL)) {
                    Success = TRUE;
                    UserContext.p = p;
                    UserContext.DataIsParam = TRUE;
                    SymEnumSymbolsExW(hProcess, 0, NULL, EnumSymbolsCallbackW,
                        &UserContext, SYMENUM_OPTIONS_DEFAULT);
                    if (p != UserContext.p) p = UserContext.p - 2;
                } else Success = FALSE;

                *p++ = ')';
                *p++ = ' ';

                DWORD Displacement;

                //
                //  StackFrame.AddrPC.Offset stores the return address rather than the caller's address.
                //  Adjust StackFrame.AddrPC.Offset by 'count' to resolve the caller's address accurately.
                //

                if (SymGetLineFromAddrW64(hProcess,
                    StackFrame.AddrPC.Offset - count, &Displacement, &Line)) {
                    memcpy(p, EXCEPTION_AT, strlen(EXCEPTION_AT));
                    size_t temp = wcslen(Line.FileName) << 1;
                    p += strlen(EXCEPTION_AT);

                    if (fConsole) {
                        memcpy(p, CONSOLE_GREEN_FORMAT, strlen(CONSOLE_GREEN_FORMAT));
                        p += strlen(CONSOLE_GREEN_FORMAT);
                    }

                    p = FormatFileLine(Line.FileName, Line.LineNumber,
                        temp, buffer + BUFLEN - p, p, fConsole);

                    if (fVerbose >= 1) p = FormatSourceCode(Line.FileName,
                        Line.LineNumber, temp, buffer + BUFLEN - p, p);
                } else {
                    memcpy(p, EXCEPTION_FROM, strlen(EXCEPTION_FROM));
                    p += strlen(EXCEPTION_FROM);

                    if (fConsole) {
                        memcpy(p, CONSOLE_GREEN_FORMAT, strlen(CONSOLE_GREEN_FORMAT));
                        p += strlen(CONSOLE_GREEN_FORMAT);
                    }

                    wchar_t Tmp[WBUFLEN];
                    len = GetModuleFileNameExW(hProcess,
                        (HMODULE) SymGetModuleBase64(hProcess, StackFrame.AddrPC.Offset), Tmp, WBUFLEN);
                    p += ConvertUnicodeToUTF8(Tmp, len << 1, p, buffer + BUFLEN - p);

                    if (fConsole) {
                        memcpy(p, CONSOLE_DEFAULT_FORMAT,
                            strlen(CONSOLE_DEFAULT_FORMAT));
                        p += strlen(CONSOLE_DEFAULT_FORMAT);
                    }

                    *p++ = '\n';
                }

                if (Success && fVerbose >= 1) {
                    UserContext.p = p;
                    UserContext.DataIsParam = FALSE;
                    SymEnumSymbolsExW(hProcess, 0, NULL, EnumSymbolsCallbackW, &UserContext, SYMENUM_OPTIONS_DEFAULT);
                    p = UserContext.p;
                }

                // Release buffer
                if (p >= buffer + (sizeof(buffer) >> 1)) {
                    WriteHandle(hStdout, buffer, p - buffer, FALSE, fConsole);
                    p = buffer;
                }

                ++count;
            }

            WriteHandle(hStdout, buffer, p - buffer, FALSE, fConsole);

            SymCleanup(hProcess);

            DWORD ErrorCode = RtlNtStatusToDosErrorNoTeb(pExceptionRecord->ExceptionCode);
            NtTerminateProcess(hProcess, ErrorCode != ERROR_MR_MID_NOT_FOUND
                                       ? ErrorCode : pExceptionRecord->ExceptionCode);
            break;
        }

        default:
            std::unreachable();
        }

        WdbgContinueDebugEvent(&StateChange, DBG_CONTINUE);
    }

    std::unreachable();
}