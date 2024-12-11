#include "main.h"

#pragma warning(suppress: 4701)
#pragma warning(suppress: 4703)
#pragma warning(suppress: 4191)
#pragma warning(suppress: 5045)
int __cdecl wmain(int argc, wchar_t *argv[])
{
    char buffer[BUFLEN];
    char *p = buffer;
    int k, timeout = 0;
    char debug = FALSE, breakpoint = TRUE, firstbreak = FALSE,
    output = TRUE, verbose = 1, start = FALSE, help = TRUE;

#if _WIN32_WINNT >= 0x0601
    ULONG UTF8StringActualByteCount;
#endif

    for (k = 1; k < argc; ++k)
    { // Handle arguments
        if (argv[k][0] != '/') break;
        if (argv[k][2] == '\0') switch(argv[k][1])
        {
            case 'B':
                breakpoint = FALSE;
                continue;
            case 'D':
                debug = TRUE;
                continue;
            case 'G':
                debug = MINGW;
                continue;
            case 'O':
                output = FALSE;
                continue;
            case 'S':
                start = TRUE;
                continue;
            case 'T':
                if (++k >= argc)
                {
                    memcpy(p, W64DBG_ERROR_INVALID, 7);
                    p += 7;
                    memcpy(p, "Value expected for '/T'\n", 24);
                    p += 24;
                } else if ((timeout = __builtin_wcstod(argv[k])) > 99999)
                {
                    memcpy(p, W64DBG_ERROR_INVALID_TIMEOUT,
                        strlen(W64DBG_ERROR_INVALID_TIMEOUT));
                    p += strlen(W64DBG_ERROR_INVALID_TIMEOUT);
                }
                continue;
            case 'V':
                verbose = 3;
                continue;
            case '?':
                memcpy(p, W64DBG_HELP, strlen(W64DBG_HELP));
                p += strlen(W64DBG_HELP);
                help = FALSE;
                continue;
        } else if (argv[k][3] == '\0')
        {
            if (argv[k][1] == 'G' && argv[k][2] == '+')
            {
                debug = MINGW + 1;
                continue;
            }
            else if (argv[k][1] == 'V' && iswdigit(argv[k][2]))
            {
                verbose = argv[k][2] - '0';
                continue;
            }
        }

        memcpy(p, W64DBG_ERROR_INVALID, 15);
        p += 15;
        memcpy(p, "argument/option - '", 19);
        p += 19;

#if _WIN32_WINNT >= 0x0601
        RtlUnicodeToUTF8N(p, buffer + BUFLEN - p,
            &UTF8StringActualByteCount, argv[k], wcslen(argv[k]) << 1);
        p += UTF8StringActualByteCount;
#else
        p += WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS,
            argv[k], -1, p, buffer + BUFLEN - p, NULL, NULL);
#endif

        memcpy(p, "'.\n", 3);
        p += 3;
    }

    if (k >= argc && help)
    { // No executable specified
        memcpy(p, W64DBG_ERROR_INVALID, 15);
        p += 15;
        memcpy(p, "syntax.\nUsage: ", 15);
        p += 15;
        memcpy(p, W64DBG_HELP + 1, 46);
        p += 46;
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
        return 1;
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

    // Check if executable exists
    if (!(PathLen = RtlDosSearchPath_U(PATH, argv[k], L".exe",
        sizeof(ApplicationName), ApplicationName, NULL)))
    {
        memcpy(buffer, W64DBG_ERROR_INVALID, 7);
        memcpy(buffer + 7, W64DBG_FILE_NOT_FOUND,
            strlen(W64DBG_FILE_NOT_FOUND));
        NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock, buffer,
            7 + strlen(W64DBG_FILE_NOT_FOUND), NULL, NULL);
        return 1;
    }

    // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefileex#syntax
    DWORD type;
    if (!GetBinaryTypeW(ApplicationName, &type) ||
        (type != SCS_32BIT_BINARY && type != SCS_64BIT_BINARY))
    { // Check if executable format (x86-64)
        memcpy(buffer, W64DBG_ERROR_INVALID, 7);

#if _WIN32_WINNT >= 0x0601
        ULONG UTF8StringActualByteCount;
        RtlUnicodeToUTF8N(buffer + 7, BUFLEN - 7,
            &UTF8StringActualByteCount, ApplicationName, PathLen);
#else
        int UTF8StringActualByteCount = WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS,
            ApplicationName, PathLen >> 1, buffer + 7, BUFLEN - 7, NULL, NULL);
#endif

        memcpy(buffer + 7 + UTF8StringActualByteCount,
            W64DBG_BAD_EXE_FORMAT, strlen(W64DBG_BAD_EXE_FORMAT));
        NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock, buffer,
            7 + strlen(W64DBG_BAD_EXE_FORMAT) + UTF8StringActualByteCount, NULL, NULL);
        return 1;
    }

    // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefileex#syntax
    size_t temp;
    wchar_t *lpCommandLine = (wchar_t *) buffer;

    // Copy arguments to pass to executable
    do
    {
        temp = wcslen(argv[k]);
        wmemcpy(lpCommandLine, argv[k], temp);
        lpCommandLine += temp;
        *lpCommandLine++ = ' ';
    } while (++k < argc);

    HANDLE hProcess;
    HANDLE hFile[MAX_DLL];
    DEBUG_EVENT DebugEvent;
    HANDLE hThread[MAX_THREAD];
    PROCESS_INFORMATION processInfo;
    LPVOID BaseOfDll[MAX_DLL] = {};
    STARTUPINFOW startupInfo = {sizeof(startupInfo)};

    *(p - 1) = '\0';

    if (debug < MINGW)
    {
        // The system closes the handles to the process and thread
        // when the debugger calls the ContinueDebugEvent function
        CreateProcessW(ApplicationName, (LPWSTR) buffer, NULL, NULL, FALSE,
            start ? PROCCREATIONFLAGS | CREATE_NEW_CONSOLE |
            DEBUG_ONLY_THIS_PROCESS : PROCCREATIONFLAGS | DEBUG_ONLY_THIS_PROCESS,
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
        CreateProcessW(ApplicationName, (LPWSTR) buffer, NULL, NULL, FALSE,
            start ? PROCCREATIONFLAGS | CREATE_NEW_CONSOLE : PROCCREATIONFLAGS,
            NULL, NULL, &startupInfo, &processInfo);

        DebugActiveProcess(processInfo.dwProcessId);
        WaitForDebugEvent(&DebugEvent, INFINITE);
        NtClose(DebugEvent.u.CreateProcessInfo.hFile);
        hThread[0] = processInfo.hThread;
        hProcess = processInfo.hProcess;
    }

    if (verbose >= 3)
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
    if ((bWow64 = !!wow64) || debug >= MINGW) --firstbreak;

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
                    DWORD Len;
                    wchar_t Tmp[WBUFLEN];

                    memcpy(buffer, "LoadDll ", 8);
                    Len = GetFinalPathNameByHandleW(DebugEvent.u.LoadDll.hFile,
                        Tmp, WBUFLEN, FILE_NAME_OPENED);

#if _WIN32_WINNT >= 0x0601
                    ULONG UTF8StringActualByteCount;
                    RtlUnicodeToUTF8N(buffer + 8, BUFLEN - 8,
                        &UTF8StringActualByteCount, Tmp, Len << 1);
#else
                    int UTF8StringActualByteCount = WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS,
                        Tmp, Len, buffer + 8, BUFLEN - 8, NULL, NULL);
#endif

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
                    if (verbose >= 3)
                    {
                        DWORD Len;
                        wchar_t Tmp[WBUFLEN];

                        memcpy(buffer, "UnloadDll ", 10);
                        Len = GetFinalPathNameByHandleW(DebugEvent.u.LoadDll.hFile,
                            Tmp, WBUFLEN, FILE_NAME_OPENED);

#if _WIN32_WINNT >= 0x0601
                        ULONG UTF8StringActualByteCount;
                        RtlUnicodeToUTF8N(buffer + 10, BUFLEN - 10,
                            &UTF8StringActualByteCount, Tmp, Len << 1);
#else
                        int UTF8StringActualByteCount = WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS,
                            Tmp, Len, buffer + 10, BUFLEN - 10, NULL, NULL);
#endif

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
                if (verbose >= 3)
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
                if (verbose >= 3)
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
                if (verbose >= 3)
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

                for (i = 0; i < MAX_DLL; ++i) if (BaseOfDll[i])
                    NtClose(hFile[i]);

                if (timeout) WaitForInputOrTimeout(hStdout, timeout);

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

                if (verbose >= 1)
                {
                    if (Console)
                    {
                        memcpy(p, "\x1b[31m", 5);
                        p += 5;
                    }

                    p = FormatVerboseDebugException(p,
                        DebugEvent.u.Exception.ExceptionRecord.ExceptionCode);

                    if (Console)
                    {
                        memcpy(p, W64DBG_UNKNOWN, 3);
                        p += 3;
                        SetConsoleMode(hStdout, ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
                    }
                }

                if (!(DebugEvent.u.Exception.ExceptionRecord.ExceptionCode & EXCEPTION_NONCONTINUABLE) &&
                    (DebugEvent.u.Exception.ExceptionRecord.ExceptionCode == 0x406D1388 ||
                    DebugEvent.u.Exception.ExceptionRecord.ExceptionCode == 0xE06D7363 || // STATUS_CPP_EH_EXCEPTION
                    DebugEvent.u.Exception.ExceptionRecord.ExceptionCode == 0xE0434f4D)) // STATUS_CLR_EXCEPTION
                    ContinueDebugEvent(DebugEvent.dwProcessId, DebugEvent.dwThreadId, DBG_CONTINUE);
                else if (debug < MINGW)
                {
                    ContinueDebugEvent(DebugEvent.dwProcessId, DebugEvent.dwThreadId, 0x80010001L);
                    NtTerminateProcess(hProcess, DebugEvent.u.Exception.ExceptionRecord.ExceptionCode);
                }
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

                        wmemcpy(&CommandLine[18 + Value.Length], L"w64dbg", 6);
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
                                "at " // attach
                                , 80, NULL, NULL);

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
                        ByteOffset.QuadPart = 80;
                        NtWriteFile(hTemp, NULL, NULL, NULL, &IoStatusBlock,
                            buffer, p - buffer + 18, &ByteOffset, NULL);
                        ByteOffset.QuadPart +=  p - buffer + 18;
                        NtSetInformationFile(hTemp, &IoStatusBlock, &ByteOffset,
                            sizeof(LARGE_INTEGER), 20); // FileEndOfFileInformation
                        NtClose(hTemp);

                        if (debug == MINGW) wmemcpy(&CommandLine[18 + Value.Length + 6], L" --batch", 8);

                        DWORD_PTR ProcDbgPt;
                        LARGE_INTEGER DelayInterval;

                        CreateProcessW(ApplicationName, CommandLine, NULL, NULL, FALSE, 
                            PROCCREATIONFLAGS, NULL, NULL, &startupInfo, &processInfo);
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

                        NtResumeProcess(hProcess);
                        // marks deletion on close, prevent writting to disk
                        NtClose(hProcess);
                        NtWaitForSingleObject(processInfo.hProcess, FALSE, NULL);
                        NtClose(processInfo.hProcess);

                        if (timeout) WaitForInputOrTimeout(hStdout, timeout);

                        if (verbose >= 3)
                        {
                            memcpy(buffer, "ExitProcess ", 12);
                            p = debug_ultoa(DebugEvent.dwProcessId, buffer + 12);
                            *p = 'x';
                            p = debug_ultoa(DebugEvent.dwThreadId, p + 1);
                            *p = '\n';
                            NtWriteFile(hStdout, NULL, NULL, NULL,
                                &IoStatusBlock, buffer, p - buffer + 1, NULL, NULL);
                        }
                        return 0;
                    } else if (verbose >= 4)
                    {
                        memcpy(p, W64DBG_ERROR_INVALID, 7);
                        p += 7;
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

#ifdef _M_ARM64
                if (processArch == IMAGE_FILE_MACHINE_ARM64 ||
                    processArch == IMAGE_FILE_MACHINE_AMD64)
                { // XXX: Unfortunate pContext is _not_ an AMD64 context, so StackWalk will fail
                    NtGetContextThread(hThread[i], &Context);
                    MachineType = IMAGE_FILE_MACHINE_ARM64;
                    StackFrame.AddrPC.Offset = Context.Pc;
                    StackFrame.AddrStack.Offset = Context.Sp;
                    StackFrame.AddrFrame.Offset = Context.Fp;
                } else
                {
                    MachineType = IMAGE_FILE_MACHINE_I386;
                    // Find no replacable NTAPI
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
#endif

                DWORD Displacement;
                char Symbol[BUFLEN];
                unsigned char count;
                PSYMBOL_INFOW pSymbol;
                IMAGEHLP_LINEW64 Line;
                DWORD64 Displacement64;
                wchar_t Tmp[WBUFLEN];
                ULONG UTF8StringActualByteCount;

                count = 0;
                pSymbol = (PSYMBOL_INFOW) Symbol;
                Line.SizeOfStruct = sizeof(Line);
                StackFrame.AddrPC.Mode = AddrModeFlat;
                StackFrame.AddrStack.Mode = AddrModeFlat;
                StackFrame.AddrFrame.Mode = AddrModeFlat;
                pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
                pSymbol->MaxNameLen = (BUFLEN - sizeof(SYMBOL_INFO)) >> 1;

                if (Console) while (TRUE)
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
                    } else
                    {
                        *p++ = '0' + count / 10;
                        *p = '0' + count % 10;
                    }

                    memcpy(++p, " \x1b[34m", 6);
                    p = ulltoaddr(StackFrame.AddrPC.Offset, p + 6, bWow64);
                    memcpy(p, W64DBG_IN, strlen(W64DBG_IN));
                    p += strlen(W64DBG_IN);

#if _WIN32_WINNT >= 0x0601
                    RtlUnicodeToUTF8N(p, buffer + BUFLEN - p,
                        &UTF8StringActualByteCount, pSymbol->Name, pSymbol->NameLen << 1);
                    p += UTF8StringActualByteCount;
#else
                    p += WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS,
                        pSymbol->Name, pSymbol->NameLen, p, buffer + BUFLEN - p, NULL, NULL);
#endif

                    memcpy(p, W64DBG_UNKNOWN, strlen(W64DBG_UNKNOWN));
                    p += strlen(W64DBG_UNKNOWN);

                    if (SymSetContext(hProcess, (PIMAGEHLP_STACK_FRAME) &StackFrame, 0))
                    {
#ifdef _M_ARM64
                        USERCONTEXT UserContext = {hProcess, ((PWOW64_CONTEXT) &Context)->Esp, p, TRUE, TRUE};
#else
                        USERCONTEXT UserContext = {hProcess, Context.Rsp, p, TRUE, TRUE};
#endif
                        SymEnumSymbolsW(hProcess, 0, NULL, EnumCallbackProc, &UserContext);
                        p = UserContext.p;
                    }
                    *p++ = ')';
                    *p++ = ' ';

                    if (debug == TRUE && SymGetLineFromAddrW64(hProcess,
                        StackFrame.AddrPC.Offset, &Displacement, &Line))
                    {
                        memcpy(p, W64DBG_AT, strlen(W64DBG_AT));
                        temp = wcslen(Line.FileName);

                        // Skip %dir%/
                        if (!wmemcmp(Line.FileName, PATH, cDirLen))
                            p = FormatFileLine(Line.FileName + cDirLen + 1,
                                Line.LineNumber,temp - cDirLen - 1, p + strlen(W64DBG_AT), TRUE);
                        else p = FormatFileLine(Line.FileName,
                            Line.LineNumber, temp, p + strlen(W64DBG_AT), TRUE);
                        if (verbose >= 2) p = FormatSourceCode(Line.FileName, Line.LineNumber, p, verbose);
                    } else
                    {
                        DWORD Len;

                        memcpy(p, W64DBG_FROM, strlen(W64DBG_FROM));
                        p += strlen(W64DBG_FROM);
                        Len = GetModuleFileNameExW(hProcess,
                            (HMODULE) SymGetModuleBase64(hProcess,
                                StackFrame.AddrPC.Offset), Tmp, WBUFLEN);

#if _WIN32_WINNT >= 0x0601
                        ULONG UTF8StringActualByteCount;
                        RtlUnicodeToUTF8N(p, buffer + BUFLEN - p,
                            &UTF8StringActualByteCount, Tmp, Len << 1);
                        p += UTF8StringActualByteCount;
#else
                        p += WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS,
                            Tmp, Len, p, buffer + BUFLEN - p, NULL, NULL);
#endif

                        memcpy(p, W64DBG_UNKNOWN, 3);
                        p += 3;
                        *p = '\n';
                        ++p;
                    }
                    ++count;
                } else while (TRUE)
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
                    } else
                    {
                        *p++ = '0' + count / 10;
                        *p = '0' + count % 10;
                    }

                    *++p = ' ';
                    p = ulltoaddr(StackFrame.AddrPC.Offset, p + 1, bWow64);
                    memcpy(p, W64DBG_IN + 3, 4);
                    p += 4;

#if _WIN32_WINNT >= 0x0601
                    RtlUnicodeToUTF8N(p, buffer + BUFLEN - p,
                        &UTF8StringActualByteCount, pSymbol->Name, pSymbol->NameLen << 1);
                    p += UTF8StringActualByteCount;
#else
                    p += WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS,
                        pSymbol->Name, pSymbol->NameLen, p, buffer + BUFLEN - p, NULL, NULL);
#endif

                    memcpy(p, W64DBG_UNKNOWN + 3, strlen(W64DBG_UNKNOWN + 3));
                    p += strlen(W64DBG_UNKNOWN + 3);

                    if (SymSetContext(hProcess, (PIMAGEHLP_STACK_FRAME) &StackFrame, 0))
                    {
#ifdef _M_ARM64
                        USERCONTEXT UserContext = {hProcess, ((PWOW64_CONTEXT) &Context)->Esp, p, FALSE, TRUE};
#else
                        USERCONTEXT UserContext = {hProcess, Context.Rsp, p, FALSE, TRUE};
#endif
                        SymEnumSymbolsW(hProcess, 0, NULL, EnumCallbackProc, &UserContext);
                        p = UserContext.p;
                    }
                    *p++ = ')';
                    *p++ = ' ';

                    if (debug == TRUE && SymGetLineFromAddrW64(hProcess,
                        StackFrame.AddrPC.Offset, &Displacement, &Line))
                    {
                        memcpy(p, W64DBG_AT, 3);
                        temp = wcslen(Line.FileName);

                        // Skip %dir%/
                        if (!wmemcmp(Line.FileName, PATH, cDirLen))
                            p = FormatFileLine(Line.FileName + cDirLen + 1,
                                Line.LineNumber, temp - cDirLen - 1, p + 3, FALSE);
                        else p = FormatFileLine(Line.FileName,
                            Line.LineNumber, temp, p + 3, FALSE);
                        if (verbose >= 2) p = FormatSourceCode(Line.FileName, Line.LineNumber, p, verbose);
                    } else
                    {
                        DWORD Len;

                        memcpy(p, W64DBG_FROM, 5);
                        p += 5;
                        Len = GetModuleFileNameExW(hProcess,
                            (HMODULE) SymGetModuleBase64(hProcess,
                                StackFrame.AddrPC.Offset), Tmp, WBUFLEN);

#if _WIN32_WINNT >= 0x0601
                        ULONG UTF8StringActualByteCount;
                        RtlUnicodeToUTF8N(p, buffer + BUFLEN - p,
                            &UTF8StringActualByteCount, Tmp, Len << 1);
                        p += UTF8StringActualByteCount;
#else
                        p += WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS,
                            Tmp, Len, p, buffer + BUFLEN - p, NULL, NULL);
#endif

                        *p++ = '\n';
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
                    NtWriteFile(hStdout, NULL, NULL, NULL,
                        &IoStatusBlock, buffer, p - buffer + 1, NULL, NULL);
                }
        }
        ContinueDebugEvent(DebugEvent.dwProcessId, DebugEvent.dwThreadId, DBG_CONTINUE);
    }
}
