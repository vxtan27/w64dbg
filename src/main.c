#include "main.rc"

#pragma warning(suppress: 4701)
#pragma warning(suppress: 4703)
#pragma warning(suppress: 4191)
#pragma warning(suppress: 5045)
int __cdecl main(int argc, char *argv[])
{
    size_t temp;
    char buffer[BUFLEN];

    char *p = buffer;
    int k, timeout = 0;
    char debug = FALSE, breakpoint = TRUE, firstbreak = FALSE,
    output = TRUE, verbose = 1, start = FALSE, help = TRUE;

    for (k = 1; k < argc; ++k)
    { // Handle arguments
        if (argv[k][0] != '/') break;
        switch(argv[k][1])
        {
            case 'B':
                if (argv[k][2] != '\0') break;
                breakpoint = FALSE;
                continue;
            case 'D':
                if (argv[k][2] != '\0') break;
                debug = TRUE;
                continue;
            case 'G':
                if (argv[k][2] == '\0')
                    debug = MINGW;
                else if (argv[k][2] == '-' && argv[k][3] == '\0')
                    debug = MINGW + 1;
                else break; // Invalid argument/option
                continue;
            case 'O':
                if (argv[k][2] != '\0') break;
                output = FALSE;
                continue;
            case 'S':
                if (argv[k][2] != '\0') break;
                start = TRUE;
                continue;
            case 'T':
                if (argv[k][2] != '\0') break;
                if (++k >= argc)
                {
                    memcpy(p, W64DBG_ERROR_INVALID, 7);
                    p += 7;
                    memcpy(p, "Value expected for '/T'\n", 24);
                    p += 24;
                } else if ((timeout = atou(argv[k])) > 99999)
                {
                    memcpy(p, W64DBG_ERROR_INVALID_TIMEOUT, 77);
                    p += 77;
                }
                continue;
            case 'V':
                if (argv[k][2] == '\0')
                    verbose = 3;
                else if (argv[k][3] == '\0')
                    verbose = argv[k][2] - '0';
                else break; // Invalid argument/option
                continue;
            case '?':
                if (argv[k][2] != '\0') break;
                memcpy(p, W64DBG_HELP, 601);
                p += 601;
                help = FALSE;
                continue;
        }

        memcpy(p, W64DBG_ERROR_INVALID, 15);
        p += 15;
        memcpy(p, "argument/option - '", 19);
        p += 19;
        temp = strlen(argv[k]);
        memcpy(p, argv[k], temp);
        p += temp;
        memcpy(p, "'.\n", 3);
        p += 3;
    }

    if (k >= argc && help)
    { // No executable specified
        memcpy(p, W64DBG_ERROR_INVALID, 15);
        p += 15;
        memcpy(p, "syntax.\nUsage: ", 15);
        p += 15;
        memcpy(p, W64DBG_HELP + 1, 50);
        p += 50;
    }

    char Console;
    HANDLE hStdout;
    OVERLAPPED Overlapped;

    Overlapped.Internal = 0;
    Overlapped.InternalHigh = 0;
    Overlapped.Offset = 0xFFFFFFFF;
    Overlapped.OffsetHigh = 0xFFFFFFFF;
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    Console = GetFileType(hStdout) == FILE_TYPE_CHAR;

    if (p != buffer)
    {
        if (Console) WriteConsole(hStdout, buffer, (DWORD) (p - buffer), NULL, NULL);
        else WriteFileEx(hStdout, buffer, (DWORD) (p - buffer),
            &Overlapped, CompletedWriteRoutine);
        return 1;
    }

    // Check if executable exists
    if (GetFileAttributesA(argv[k]) == INVALID_FILE_ATTRIBUTES)
    {
        memcpy(buffer, W64DBG_ERROR_INVALID, 7);
        memcpy(buffer + 7, W64DBG_FILE_NOT_FOUND, 43);
        if (Console) WriteConsole(hStdout, buffer, 50, NULL, NULL);
        else WriteFileEx(hStdout, buffer, 50, &Overlapped, CompletedWriteRoutine);
        return 1;
    }

    // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefileex#syntax
    LPDWORD type = (LPDWORD) &Overlapped.hEvent;
    if (GetBinaryTypeA(argv[k], type) == 0 ||
        (*type != SCS_32BIT_BINARY && *type != SCS_64BIT_BINARY))
    { // Check if executable format (x86-64)
        memcpy(buffer, W64DBG_ERROR_INVALID, 7);
        temp = strlen(argv[k]);
        memcpy(buffer + 7, argv[k], temp);
        memcpy(buffer + 7 + temp, W64DBG_BAD_EXE_FORMAT, 35);
        if (Console) WriteConsole(hStdout, buffer, (DWORD)(42 + temp), NULL, NULL);
        else WriteFileEx(hStdout, buffer, (DWORD)(42 + temp), &Overlapped, CompletedWriteRoutine);
        return 1;
    }

    // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefileex#syntax
    Overlapped.hEvent = argv[k];
    p = buffer;

    // Copy arguments to pass to executable
    do
    {
        temp = strlen(argv[k]);
        memcpy(p, argv[k], temp);
        p += (DWORD) temp;
        *p++ = ' ';
    } while (++k < argc);

    HANDLE hProcess;
    HANDLE hFile[MAX_DLL];
    DEBUG_EVENT DebugEvent;
    HANDLE hThread[MAX_THREAD];
    PROCESS_INFORMATION processInfo;
    LPVOID lpBaseOfDll[MAX_DLL] = {};
    STARTUPINFO startupInfo = {sizeof(startupInfo)};

    *(p - 1) = '\0';

    if (debug >= MINGW)
    {
        // The system closes the handles to the process and thread
        // when the debugger calls the DebugActiveProcessStop function
        if (start) CreateProcessA((LPCSTR) Overlapped.hEvent, buffer, NULL, NULL, FALSE,
            CREATE_PRESERVE_CODE_AUTHZ_LEVEL | CREATE_NEW_CONSOLE,
            NULL, NULL, &startupInfo, &processInfo);
        else CreateProcessA((LPCSTR) Overlapped.hEvent, buffer, NULL, NULL, FALSE,
            CREATE_PRESERVE_CODE_AUTHZ_LEVEL,
        NULL, NULL, &startupInfo, &processInfo);

        DebugActiveProcess(processInfo.dwProcessId);
        WaitForDebugEvent(&DebugEvent, INFINITE);
        CloseHandle(DebugEvent.u.CreateProcessInfo.hFile);
        hThread[0] = processInfo.hThread;
        hProcess = processInfo.hProcess;
    } else
    {
        // The system closes the handles to the process and thread
        // when the debugger calls the ContinueDebugEvent function
        if (start) CreateProcessA((LPCSTR) Overlapped.hEvent, buffer, NULL, NULL, FALSE,
            CREATE_PRESERVE_CODE_AUTHZ_LEVEL | CREATE_NEW_CONSOLE | DEBUG_ONLY_THIS_PROCESS,
            NULL, NULL, &startupInfo, &processInfo);
        else CreateProcessA((LPCSTR) Overlapped.hEvent, buffer, NULL, NULL, FALSE,
            CREATE_PRESERVE_CODE_AUTHZ_LEVEL | DEBUG_ONLY_THIS_PROCESS,
            NULL, NULL, &startupInfo, &processInfo);

        CloseHandle(processInfo.hThread);
        CloseHandle(processInfo.hProcess);
        WaitForDebugEvent(&DebugEvent, INFINITE);
        if (debug == TRUE)
        {
            hFile[0] = DebugEvent.u.CreateProcessInfo.hFile;
            lpBaseOfDll[0] = DebugEvent.u.CreateProcessInfo.lpBaseOfImage;
        } else CloseHandle(DebugEvent.u.CreateProcessInfo.hFile);
        hProcess = DebugEvent.u.CreateProcessInfo.hProcess;
        hThread[0] = DebugEvent.u.CreateProcessInfo.hThread;
    }

    if (verbose >= 3)
    {
        memcpy(buffer, "CreateProcess ", 14);
        p = debug_ultoa(DebugEvent.dwProcessId, buffer + 14);
        *p = 'x';
        p = debug_ultoa(DebugEvent.dwThreadId, p + 1);
        *p = '\n';
        if (Console) WriteConsole(hStdout, buffer,
            (DWORD) (p - buffer) + 1, NULL, NULL);
        else WriteFileEx(hStdout, buffer,
            (DWORD) (p - buffer) + 1, &Overlapped, CompletedWriteRoutine);
    }

    ContinueDebugEvent(DebugEvent.dwProcessId, DebugEvent.dwThreadId, DBG_CONTINUE);

    unsigned char i;
    DWORD dwThreadId[MAX_THREAD] = {};

    BOOL bWow64;
#ifndef _M_ARM64
    IsWow64Process(hProcess, &bWow64);
    if (bWow64 || debug >= MINGW) --firstbreak;
#else
    USHORT processArch, nativeArch;
    IsWow64Process2(hProcess, &processArch, &nativeArch);
    if (processArch == IMAGE_FILE_MACHINE_UNKNOWN)
    {
        PROCESS_MACHINE_INFORMATION processMachineInfo;
        if (GetProcessInformation(hProcess, ProcessMachineTypeInfo, &processMachineInfo, sizeof(processMachineInfo)))
            processArch = processMachineInfo.ProcessMachine;
        else processArch = nativeArch;
    }
    if (processArch == IMAGE_FILE_MACHINE_I386)
    {
        bWow64 = TRUE;
        if (debug >= MINGW) --firstbreak;
    }
#endif

    dwThreadId[0] = DebugEvent.dwThreadId;

    while (TRUE)
    {
        WaitForDebugEvent(&DebugEvent, INFINITE);
        // https://learn.microsoft.com/en-us/windows/win32/debug/debugging-events
        switch (DebugEvent.dwDebugEventCode)
        {

            case LOAD_DLL_DEBUG_EVENT:
                if (verbose >= 3)
                {
                    DWORD len;

                    memcpy(buffer, "LoadDll ", 8);
                    len = GetFinalPathNameByHandleA(DebugEvent.u.LoadDll.hFile,
                        buffer + 8, BUFLEN, FILE_NAME_OPENED);
                    buffer[len + 8] = '\n';
                    if (Console) WriteConsole(hStdout, buffer, len + 9,
                        NULL, NULL);
                    else WriteFileEx(hStdout, buffer, len + 9,
                        &Overlapped, CompletedWriteRoutine);
                }

                // Find storage position
                for (i = 0; i < MAX_DLL; ++i) if (!lpBaseOfDll[i])
                {
                    hFile[i] = DebugEvent.u.LoadDll.hFile;
                    lpBaseOfDll[i] = DebugEvent.u.LoadDll.lpBaseOfDll;
                    break;
                }
                break;

            case UNLOAD_DLL_DEBUG_EVENT:
                // Find specific DLL
                for (i = 0; i < MAX_DLL; ++i)
                    if (DebugEvent.u.UnloadDll.lpBaseOfDll == lpBaseOfDll[i])
                {
                    CloseHandle(hFile[i]);
                    if (verbose >= 3)
                    {
                        DWORD len;

                        memcpy(buffer, "UnloadDll ", 10);
                        len = GetFinalPathNameByHandleA(hFile[i],
                            buffer + 10, BUFLEN, FILE_NAME_OPENED);
                        buffer[len + 10] = '\n';
                        if (Console) WriteConsole(hStdout, buffer, len + 11,
                            NULL, NULL);
                        else WriteFileEx(hStdout, buffer, len + 11,
                            &Overlapped, CompletedWriteRoutine);
                    }
                    lpBaseOfDll[i] = 0;
                    break;
                }
                break;

            case CREATE_THREAD_DEBUG_EVENT:
                if (verbose >= 3)
                {
                    memcpy(buffer, "CreateThread ", 13);
                    p = debug_ultoa(DebugEvent.dwProcessId, buffer + 13);
                    *p = 'x';
                    p = debug_ultoa(DebugEvent.dwThreadId, p + 1);
                    *p = '\n';
                    if (Console) WriteConsole(hStdout, buffer,
                        (DWORD) (p - buffer) + 1, NULL, NULL);
                    else WriteFileEx(hStdout, buffer, (DWORD) (p - buffer) + 1,
                        &Overlapped, CompletedWriteRoutine);
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
                if (verbose >= 3)
                {
                    memcpy(buffer, "ExitThread ", 11);
                    p = debug_ultoa(DebugEvent.dwProcessId, buffer + 11);
                    *p = 'x';
                    p = debug_ultoa(DebugEvent.dwThreadId, p + 1);
                    *p = '\n';
                    if (Console) WriteConsole(hStdout, buffer,
                        (DWORD) (p - buffer) + 1, NULL, NULL);
                    else WriteFileEx(hStdout, buffer, (DWORD) (p - buffer) + 1,
                        &Overlapped, CompletedWriteRoutine);
                }

                // Find specific thread
                for (i = 0; i < MAX_THREAD; ++i) if (DebugEvent.dwThreadId == dwThreadId[i])
                {
                    dwThreadId[i] = 0;
                    break;
                }
                break;

            case EXIT_PROCESS_DEBUG_EVENT:
                if (verbose >= 3)
                {
                    memcpy(buffer, "ExitProcess ", 12);
                    p = debug_ultoa(DebugEvent.dwProcessId, buffer + 12);
                    *p = 'x';
                    p = debug_ultoa(DebugEvent.dwThreadId, p + 1);
                    *p = '\n';
                    if (Console) WriteConsole(hStdout, buffer,
                        (DWORD) (p - buffer) + 1, NULL, NULL);
                    else WriteFileEx(hStdout, buffer, (DWORD) (p - buffer) + 1,
                        &Overlapped, CompletedWriteRoutine);
                }

                // Cleanup
                if (debug >= MINGW)
                {
                    CloseHandle(hProcess);
                    CloseHandle(hThread[0]);
                }

                for (i = 0; i < MAX_DLL; ++i) if (lpBaseOfDll[i])
                    CloseHandle(hFile[i]);

                if (timeout) WaitForInputOrTimeout(hStdout, timeout, Console, &Overlapped);

                ContinueDebugEvent(DebugEvent.dwProcessId, DebugEvent.dwThreadId, DBG_CONTINUE);
                return 0;

            case OUTPUT_DEBUG_STRING_EVENT:
                if (verbose >= 3)
                {
                    memcpy(buffer, "OutputDebugString ", 18);
                    p = debug_ultoa(DebugEvent.dwProcessId, buffer + 18);
                    *p = 'x';
                    p = debug_ultoa(DebugEvent.dwThreadId, p + 1);
                    *p = '\n';
                    if (Console) WriteConsole(hStdout, buffer,
                        (DWORD) (p - buffer) + 1, NULL, NULL);
                    else WriteFileEx(hStdout, buffer, (DWORD) (p - buffer) + 1,
                        &Overlapped, CompletedWriteRoutine);
                }

                if (output == TRUE)
                {
                    SIZE_T TEMP;

                    NtReadVirtualMemory(hProcess,
                        DebugEvent.u.DebugString.lpDebugStringData,
                        buffer, DebugEvent.u.DebugString.nDebugStringLength - 1, // exclude '\0'
                        &TEMP);

                    if (Console) WriteConsole(hStdout, buffer,
                        (DWORD) TEMP, NULL, NULL);
                    else WriteFileEx(hStdout, buffer, (DWORD) TEMP,
                        &Overlapped, CompletedWriteRoutine);
                }
                break;

            case EXCEPTION_DEBUG_EVENT:
                // Skip thread naming exception
                if (DebugEvent.u.Exception.ExceptionRecord.ExceptionCode == 0x406D1388)
                    break;
                // Skip first-chance breakpoints
                if ((DebugEvent.u.Exception.ExceptionRecord.ExceptionCode == 0x80000003 || // EXCEPTION_BREAKPOINT
                    DebugEvent.u.Exception.ExceptionRecord.ExceptionCode == 0x4000001F ) && // STATUS_WX86_BREAKPOINT
                    ((breakpoint == FALSE) || (breakpoint == TRUE && ++firstbreak <= 1)))
                    break;

                // Find thread where exception occured
                for (i = 0; i < MAX_THREAD; ++i) if (DebugEvent.dwThreadId == dwThreadId[i])
                    break;

                if (DebugEvent.u.Exception.dwFirstChance == 0 ||
                    DebugEvent.dwThreadId != dwThreadId[i])
                { // Check if first-chance or cannot find thread
                    ContinueDebugEvent(DebugEvent.dwProcessId,
                        DebugEvent.dwThreadId, DBG_EXCEPTION_NOT_HANDLED);
                    continue;
                }

                memcpy(buffer, "Thread #", 8);
                buffer[8] = '0' + (i + 1) / 10;
                buffer[9] = '0' + (i + 1) % 10;
                memcpy(buffer + 10, " caused ", 8);
                p = FormatDebugException(&DebugEvent.u.Exception.ExceptionRecord, buffer + 18, bWow64);
                *p++ = '\n';

                if (verbose >= 1)
                {
                    if (Console)
                    {
                        memcpy(p, "\x1b[31m", 5);
                        p += 5;
                    }

                    p = FormatVerboseDebugException(p,
                        DebugEvent.u.Exception.ExceptionRecord.ExceptionCode);
                    *p++ = '\n';

                    if (Console)
                    {
                        memcpy(p, W64DBG_UNKNOWN, 3);
                        p += 3;
                        SetConsoleMode(hStdout, ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
                    }
                }

                if (debug >= MINGW)
                {
                    char gdb[MAX_PATH];

                    if (SearchPathA(NULL, "gdb.exe", NULL, MAX_PATH, gdb, NULL))
                    {
                        NtSuspendProcess(hProcess);
                        DebugActiveProcessStop(DebugEvent.dwProcessId);

                        if (Console) WriteConsole(hStdout, buffer, (DWORD) (p - buffer), NULL, NULL);
                        else WriteFileEx(hStdout, buffer, (DWORD) (p - buffer),
                            &Overlapped, CompletedWriteRoutine);

                        DWORD len;
                        HANDLE hTemp;
                        char cmd[64] = "gdb.exe -q -x ";

                        len = GetTempPath(MAX_PATH + 1, &cmd[14]);
                        memcpy(&cmd[14 + len], ".gdbinit", 8);
                        hTemp = CreateFileA(&cmd[14], GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
                            FILE_FLAG_OVERLAPPED , NULL);

                        if (GetLastError() != ERROR_ALREADY_EXISTS)
                        { // First time run
                            memcpy(buffer,
                                "set bac l 100\n" // set backtrace limit 100
                                "set p th of\n" // set print thread-events off
                                "set p i of\n" // set print inferior-events off
                                "set lo ov\n" // set logging overwrite
                                "set lo r\n" // set logging redirect
                                "set lo d\n" // set logging debugredirect
                                "set lo f " // set logging file
                                , 74);
                            memcpy(buffer + 74, &cmd[14], len);
                            // set logging enabled & attach
                            memcpy(buffer + 74 + len, "gdb.txt\nset lo e\nat ", 20);
                            WriteFileEx(hTemp, buffer, 74 + len + 20,
                                &Overlapped, CompletedWriteRoutine);
                        } else
                        {
                            Overlapped.OffsetHigh = 0;
                            Overlapped.Offset = 74 + len + 20;
                        }

                        space_ultoa(DebugEvent.dwProcessId, buffer);
                        p = buffer + 5;

                        if (debug == MINGW + 1)
                        {
                            memcpy(p,
                                "\nset con of" // set confirm off
                                "\nset pa of" // set pagination off
                                "\nset wi 0" // set width 0
                                , 30);
                            p += 30;
                        } else
                        {
                            // set style enabled
                            memcpy(p, "\nset sty e", 10);
                            p += 10;
                        }

                        if (verbose < 3)
                        {
                            // set print entry-values no
                            memcpy(p, "\nset p en n", 11);
                            p += 11;
                        }

                        // continue & set logging enabled off & backtrace
                        memcpy(p, "\nc\nset lo e of\nbt\n", 18);
                        WriteFileEx(hTemp, buffer, (DWORD)(p - buffer) + 18,
                            &Overlapped, CompletedWriteRoutine);
                        SetFilePointer(hTemp, 74 + len + 20 + (DWORD)(p - buffer) + 18, NULL, FILE_BEGIN);
                        SetEndOfFile(hTemp);
                        CloseHandle(hTemp);

                        if (debug == MINGW) memcpy(&cmd[14 + len + 8], " --batch", 8);

                        unsigned char beingDebugged;
                        PROCESS_BASIC_INFORMATION ProcessBasicInfo;

                        CreateProcessA(gdb, cmd, NULL, NULL, TRUE,
                            CREATE_PRESERVE_CODE_AUTHZ_LEVEL,
                            NULL, NULL, &startupInfo, &processInfo);
                        CloseHandle(processInfo.hThread);
                        CloseHandle(hThread[0]);

                        while (TRUE)
                        {
                            SleepEx(LATENCY, FALSE);
                            NtQueryInformationProcess(hProcess, ProcessBasicInformation,
                                &ProcessBasicInfo, sizeof(ProcessBasicInfo), NULL);
                            NtReadVirtualMemory(hProcess, (char *) ProcessBasicInfo.PebBaseAddress + 2,
                                &beingDebugged, 1, NULL);
                            if (beingDebugged) break;

                        }

                        NtResumeProcess(hProcess);
                        // marks deletion on close, prevent writting to disk
                        CloseHandle(hProcess);
                        Overlapped.Offset = 0xFFFFFFFF;
                        Overlapped.OffsetHigh = 0xFFFFFFFF;
                        WaitForSingleObjectEx(processInfo.hProcess, INFINITE, FALSE);
                        CloseHandle(processInfo.hProcess);

                        if (timeout) WaitForInputOrTimeout(hStdout, timeout, Console, &Overlapped);

                        if (verbose >= 3)
                        {
                            memcpy(buffer, "ExitProcess ", 12);
                            p = debug_ultoa(DebugEvent.dwProcessId, buffer + 12);
                            *p = 'x';
                            p = debug_ultoa(DebugEvent.dwThreadId, p + 1);
                            *p = '\n';
                            if (Console) WriteConsole(hStdout, buffer, (DWORD) (p - buffer) + 1, NULL, NULL);
                            else WriteFileEx(hStdout, buffer, (DWORD) (p - buffer) + 1,
                                &Overlapped, CompletedWriteRoutine);
                        }
                        return 0;
                    } else if (verbose >= 4)
                    {
                        memcpy(p, W64DBG_ERROR_INVALID, 7);
                        p += 7;
                        memcpy(p, W64DBG_FILE_NOT_FOUND, 43);
                        p += 43;
                    }
                }

                DWORD MachineType;
                STACKFRAME64 StackFrame;

                SymSetOptions(debug == TRUE ? _DebugSymOptions : NDebugSymOptions);
                SymInitialize(hProcess, NULL, FALSE);

                for (unsigned char j = 0; j < MAX_DLL; ++j) if (lpBaseOfDll[j])
                    SymLoadModuleEx(hProcess,
                        hFile[j],
                        NULL,
                        NULL,
                        (DWORD64) lpBaseOfDll[j],
                        GetFileSize(hFile[j], NULL),
                        NULL,
                        0);

                //memset(&StackFrame, 0, sizeof(StackFrame));

                CONTEXT Context;

#ifdef _M_ARM64
                if (processArch == IMAGE_FILE_MACHINE_ARM64 ||
                    processArch == IMAGE_FILE_MACHINE_AMD64)
                { // XXX: Unfortunate pContext is _not_ an AMD64 context, so StackWalk will fail
                    GetThreadContext(hThread[i], &Context);
                    MachineType = IMAGE_FILE_MACHINE_ARM64;
                    StackFrame.AddrPC.Offset = Context.Pc;
                    StackFrame.AddrStack.Offset = Context.Sp;
                    StackFrame.AddrFrame.Offset = Context.Fp;
                } else
                {
                    MachineType = IMAGE_FILE_MACHINE_I386;
                    Wow64GetThreadContext(hThread[i], (PWOW64_CONTEXT) &Context);
                    StackFrame.AddrPC.Offset = ((PWOW64_CONTEXT) &Context)->Eip;
                    StackFrame.AddrStack.Offset = ((PWOW64_CONTEXT) &Context)->Esp;
                    StackFrame.AddrFrame.Offset = ((PWOW64_CONTEXT) &Context)->Ebp;
                }
#else
                if (bWow64)
                {
                    ((PWOW64_CONTEXT) &Context)->ContextFlags = WOW64_CONTEXT_ALL;
                    Wow64GetThreadContext(hThread[i], (PWOW64_CONTEXT) &Context);
                    MachineType = IMAGE_FILE_MACHINE_I386;
                    StackFrame.AddrPC.Offset = ((PWOW64_CONTEXT) &Context)->Eip;
                    StackFrame.AddrStack.Offset = ((PWOW64_CONTEXT) &Context)->Esp;
                    StackFrame.AddrFrame.Offset = ((PWOW64_CONTEXT) &Context)->Ebp;
                } else
                {
                    Context.ContextFlags = CONTEXT_ALL;
                    GetThreadContext(hThread[i], &Context);
                    MachineType = IMAGE_FILE_MACHINE_AMD64;
                    StackFrame.AddrPC.Offset = Context.Rip;
                    StackFrame.AddrStack.Offset = Context.Rsp;
                    StackFrame.AddrFrame.Offset = Context.Rbp;
                }
#endif

                DWORD cDirLen;
                char cDir[BUFLEN];
                DWORD Displacement;
                char Symbol[BUFLEN];
                unsigned char count;
                IMAGEHLP_LINE64 Line;
                DWORD64 Displacement64;
                PSYMBOL_INFO pSymbol;

                count = 0;
                pSymbol = (PSYMBOL_INFO) Symbol;
                Line.SizeOfStruct = sizeof(Line);
                StackFrame.AddrPC.Mode = AddrModeFlat;
                StackFrame.AddrStack.Mode = AddrModeFlat;
                StackFrame.AddrFrame.Mode = AddrModeFlat;
                pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
                pSymbol->MaxNameLen = BUFLEN - sizeof(SYMBOL_INFO);
                cDirLen = GetCurrentDirectory(BUFLEN, cDir);

                if (Console)
                {
                    while (TRUE)
                    {
                        if (!StackWalk64(MachineType, hProcess, hThread[i], &StackFrame,
                            &Context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
                            break;

                        if (!SymFromAddr(hProcess, StackFrame.AddrPC.Offset, &Displacement64, pSymbol))
                            break;

                        *p++ = '#';

                        if (count < 10)
                        {
                            *p++ = '0' + count;
                            *p = ' ';
                        } else
                        {
                            *p++ = '0' + count / 10;
                            *p = '0' + count % 10;
                        }

                        memcpy(++p, " \x1b[34m", 6);
                        p = ulltoaddr((ULONGLONG) StackFrame.AddrPC.Offset, p + 6, bWow64);
                        memcpy(p, W64DBG_IN, 12);
                        p += 12;
                        memcpy(p, pSymbol->Name, pSymbol->NameLen);
                        p += pSymbol->NameLen;
                        memcpy(p, W64DBG_UNKNOWN, 5);
                        p += 5;

                        if (SymSetContext(hProcess, (PIMAGEHLP_STACK_FRAME) &StackFrame, 0))
                        {
#ifdef _M_ARM64
                            USERCONTEXT UserContext = {hProcess, ((PWOW64_CONTEXT) &Context)->Esp, p, Console, TRUE};
#else
                            USERCONTEXT UserContext = {hProcess, Context.Rsp, p, Console, TRUE};
#endif
                            SymEnumSymbols(hProcess, 0, NULL, EnumCallbackProc, &UserContext);
                            p = UserContext.p;
                        }
                        *p++ = ')';
                        *p++ = ' ';

                        if (debug == TRUE && SymGetLineFromAddr64(hProcess,
                            StackFrame.AddrPC.Offset, &Displacement, &Line))
                        {
                            memcpy(p, W64DBG_AT, 8);
                            temp = strlen(Line.FileName);

                            // Skip %dir%/
                            if (memcmp(Line.FileName, cDir, cDirLen) == 0)
                                p = FormatFileLine(Line.FileName + cDirLen + 1,
                                    Line.LineNumber, temp - cDirLen - 1, p + 8, TRUE);
                            else p = FormatFileLine(Line.FileName,
                                Line.LineNumber, temp, p + 8, TRUE);
                            if (verbose >= 2) p = FormatSourceCode(Line.FileName, Line.LineNumber, p, verbose);
                        } else
                        {
                            memcpy(p, W64DBG_FROM, 10);
                            p += 10;
                            p += GetModuleFileNameExA(hProcess,
                                (HMODULE) SymGetModuleBase64(hProcess,
                                    StackFrame.AddrPC.Offset), p, BUFLEN);
                            memcpy(p, W64DBG_UNKNOWN, 3);
                            p += 3;
                            *p = '\n';
                            ++p;
                        }
                        ++count;
                    }
                } else
                {

                    while (TRUE)
                    {
                        if (!StackWalk64(MachineType, hProcess, hThread[i], &StackFrame,
                            &Context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
                            break;
                        if (!SymFromAddr(hProcess, StackFrame.AddrPC.Offset, &Displacement64, pSymbol))
                            break;

                        *p++ = '#';
                        if (count < 10)
                        {
                            *p++ = '0' + count;
                            *p = ' ';
                        } else
                        {
                            *p++ = '0' + count / 10;
                            *p = '0' + count % 10;
                        }

                        *++p = ' ';
                        p = ulltoaddr((ULONGLONG) StackFrame.AddrPC.Offset, p + 1, bWow64);
                        memcpy(p, W64DBG_IN + 3, 4);
                        p += 4;
                        memcpy(p, pSymbol->Name, pSymbol->NameLen);
                        p += pSymbol->NameLen;
                        memcpy(p, W64DBG_UNKNOWN + 3, 2);
                        p += 2;

                        if (SymSetContext(hProcess, (PIMAGEHLP_STACK_FRAME) &StackFrame, 0))
                        {
#ifdef _M_ARM64
                            USERCONTEXT UserContext = {hProcess, ((PWOW64_CONTEXT) &Context)->Esp, p, Console, TRUE};
#else
                            USERCONTEXT UserContext = {hProcess, Context.Rsp, p, Console, TRUE};
#endif
                            SymEnumSymbols(hProcess, 0, NULL, EnumCallbackProc, &UserContext);
                            p = UserContext.p;
                        }
                        *p++ = ')';
                        *p++ = ' ';

                        if (debug == TRUE && SymGetLineFromAddr64(hProcess,
                            StackFrame.AddrPC.Offset, &Displacement, &Line))
                        {
                            memcpy(p, W64DBG_AT, 3);
                            temp = strlen(Line.FileName);

                            // Skip %dir%/
                            if (memcmp(Line.FileName, cDir, cDirLen) == 0)
                                p = FormatFileLine(Line.FileName + cDirLen + 1,
                                    Line.LineNumber, temp - cDirLen - 1, p + 3, FALSE);
                            else p = FormatFileLine(Line.FileName,
                                Line.LineNumber, temp, p + 3, FALSE);
                            if (verbose >= 2) p = FormatSourceCode(Line.FileName, Line.LineNumber, p, verbose);
                        } else
                        {
                            memcpy(p, W64DBG_FROM, 5);
                            p += 5;
                            p += GetModuleFileNameExA(hProcess,
                                (HMODULE) SymGetModuleBase64(hProcess,
                                    StackFrame.AddrPC.Offset), p, BUFLEN);
                            *p++ = '\n';
                        }
                        ++count;
                    }
                }

                for (i = 0; i < MAX_DLL; ++i) if (lpBaseOfDll[i])
                    SymUnloadModule64(hProcess,
                        (DWORD64) lpBaseOfDll[i]);

                SymCleanup(hProcess);
                TerminateProcess(hProcess, DebugEvent.u.Exception.ExceptionRecord.ExceptionCode);
                ContinueDebugEvent(DebugEvent.dwProcessId,
                    DebugEvent.dwThreadId, DBG_EXCEPTION_NOT_HANDLED);

                if (Console) WriteConsole(hStdout, buffer, (DWORD) (p - buffer), NULL, NULL);
                else WriteFileEx(hStdout, buffer, (DWORD) (p - buffer),
                    &Overlapped, CompletedWriteRoutine);
                continue;

            case RIP_EVENT:
                if (verbose >= 3)
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

                    if (Console) WriteConsole(hStdout, buffer,
                        (DWORD) (p - buffer) + 1, NULL, NULL);
                    else WriteFileEx(hStdout, buffer, (DWORD) (p - buffer) + 1,
                        &Overlapped, CompletedWriteRoutine);
                }
        }
        ContinueDebugEvent(DebugEvent.dwProcessId, DebugEvent.dwThreadId, DBG_CONTINUE);
    }
}