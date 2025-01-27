/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

#include "core.h"

// https://hero.handmade.network/forums/code-discussion/t/94-guide_-_how_to_avoid_c_c++_runtime_on_windows

__declspec(noreturn)
void __stdcall main(void)
{
    size_t temp, len;
    char buffer[BUFLEN];
    ULONG UTF8StringActualByteCount;

    long timeout = DEFAULT_TIMEOUT;
    char breakpoint = DEFAULT_BREAKPOINT,
    firstbreak = DEFAULT_FIRSTBREAK,
    dwarf = DEFAULT_DEBUG_DWARF,
    verbose = DEFAULT_VERBOSE,
    output = DEFAULT_OUTPUT,
    start = DEFAULT_START,
    help = DEFAULT_HELP;

    char* p = buffer;
    PZWRTL_USER_PROCESS_PARAMETERS ProcessParameters = ((PPEB) __readgsqword(0x60))->ProcessParameters;
    len = ProcessParameters->CommandLine.Length >> 1;
    PWSTR pCmdLine = wmemchr(ProcessParameters->CommandLine.Buffer, ' ', len);
    PWSTR pNext = pCmdLine;

    if (pCmdLine)
    {
        len -= pCmdLine - ProcessParameters->CommandLine.Buffer;
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
                    } else if (*(pNext + 2) >= '0' &&
                        *(pNext + 2) <= '9' && *(pNext + 3) == ' ')
                    {
                        breakpoint -= *(pNext + 2) - '0';
                        pNext += 4;
                    } else break;

                    continue;

                case 'D':
                case 'd':
                    if (*(pNext + 2) == ' ')
                    {
                        dwarf = FALSE;
                        pNext += 3;
                        continue;
                    }

                case 'G':
                case 'g':
                    if (*(pNext + 2) == ' ')
                    {
                        dwarf = MINGW_KEEP;
                        pNext += 3;
                    } else if (*(pNext + 2) == '+' && *(pNext + 3) == ' ')
                    {
                        dwarf = MINGW_NOKEEP;
                        pNext += 4;
                    } else break;

                    continue;

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
                    wchar_t* cmd = pNext;
                    pNext += 2;

                    while (*pNext == ' ') ++pNext; // Skip spaces

                    temp = pCmdLine + len + 1 - pNext;

                    if (temp <= 0)
                    {
                        memcpy(p, VALUE_EXPECTED,
                            sizeof(VALUE_EXPECTED));
                        p += sizeof(VALUE_EXPECTED);
                        *p++ = *cmd;
                        *p++ = *(cmd + 1);
                        *p++ = '\'';
                        *p++ = '\n';
                    } else
                    {
                        if ((timeout = process_timeout(pNext, &pNext, temp)) > VALID_TIMEOUT)
                        {
                            memcpy(p, TIMEOUT_INVALID, sizeof(TIMEOUT_INVALID));
                            p += sizeof(TIMEOUT_INVALID);
                            *(p - 43) = *cmd;
                            *(p - 42) = *(cmd + 1);
                        }
                    }

                    continue;

                case 'V':
                case 'v':
                    if (*(pNext + 2) == ' ')
                    {
                        verbose = 3;
                        pNext += 3;
                    } else if (*(pNext + 2) >= '0' &&
                        *(pNext + 2) <= '9' && *(pNext + 3) == ' ')
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

            memcpy(p, _INVALID_ARGUMENT,
                sizeof(_INVALID_ARGUMENT));
            p += sizeof(_INVALID_ARGUMENT);

            temp = (wchar_t*) wmemchr(pNext, ' ',
                pCmdLine + len + 1 - pNext) - pNext;

            RtlUnicodeToUTF8N(p, buffer + BUFLEN - p,
                &UTF8StringActualByteCount, pNext, temp << 1);
            p += UTF8StringActualByteCount;
            pNext += temp + 1;

            memcpy(p, INVALID_ARGUMENT_, 3);
            p += 3;
        }
    }

    if (help)
    { // help message
        memcpy(p, HELP + 16, sizeof(HELP) - 16);
        p += sizeof(HELP) - 16;
    } else if (!pCmdLine || pCmdLine + len < pNext)
    { // No executable specified
        memcpy(p, HELP, 65);
        p += 65;
    }

    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE hStdout = ProcessParameters->StandardOutput;
    char Console = GetFileType(hStdout) == FILE_TYPE_CHAR;

    if (Console)
    {
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleMode(hStdout, ENABLE_PROCESSED_OUTPUT |
            ENABLE_WRAP_AT_EOL_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }

    if (p != buffer)
    {
        NtWriteFile(hStdout, NULL, NULL, NULL,
            &IoStatusBlock, buffer, p - buffer, NULL, NULL);
        RtlExitUserProcess(1);
    }

    wchar_t* ptr;
    DWORD wDirLen;
    wchar_t PATH[WBUFLEN], ApplicationName[WBUFLEN];

    wDirLen = RtlGetCurrentDirectory_U(sizeof(PATH), PATH) >> 1;
    PATH[wDirLen] = ';';
    RtlQueryEnvironmentVariable(ProcessParameters->Environment, PATHENV,
        sizeof(PATHENV) >> 1, PATH + wDirLen + 1, WBUFLEN - wDirLen - 1, &temp);

    ptr = (wchar_t*) wmemchr(pNext, ' ',
        pCmdLine + len + 1 - pNext);
    *ptr = '\0';

    *(pCmdLine + len) = '\0';

    if (!RtlDosSearchPath_U(PATH, pNext, EXTENSION,
        sizeof(ApplicationName), ApplicationName, NULL))
    { // Check if executable exists
        MESSAGE_RESOURCE_ENTRY *Entry;

        FindSystemMessage(ERROR_FILE_NOT_FOUND, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &Entry);
        // Convert error message to UTF-8
        RtlUnicodeToUTF8N(buffer, BUFLEN,
            &UTF8StringActualByteCount, Entry->Text, Entry->Length - 8);
        NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock,
            buffer, UTF8StringActualByteCount, NULL, NULL);
        RtlExitUserProcess(1);
    }

    DWORD bx64win; // Is 64-bit application

    if (!GetBinaryTypeW(ApplicationName, &bx64win) ||
        (bx64win != SCS_32BIT_BINARY && bx64win != SCS_64BIT_BINARY))
    { // Check if executable format (x86-64)
        wchar_t* pos;
        MESSAGE_RESOURCE_ENTRY *Entry;

        FindSystemMessage(ERROR_BAD_EXE_FORMAT, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &Entry);

        // Convert error message to UTF-8
        if (*Entry->Text == '%')
        {
            pos = Entry->Text;
            p = buffer;
        }
        else
        {
            // Should - 8 and () >> 1
            pos = (wchar_t*) wmemchr(Entry->Text, '%', Entry->Length);
            RtlUnicodeToUTF8N(buffer, BUFLEN, &UTF8StringActualByteCount,
                Entry->Text, pos - Entry->Text);
            p = buffer + UTF8StringActualByteCount;
        }

        // Convert filename to UTF-8
        RtlUnicodeToUTF8N(p, buffer + BUFLEN - p, &UTF8StringActualByteCount,
            pNext, (ptr - pNext) << 1);
        p += UTF8StringActualByteCount;

        pos = (wchar_t*) wmemchr(pos + 1, '1', Entry->Length) + 1;
        // Convert error message to UTF-8
        RtlUnicodeToUTF8N(p, buffer + BUFLEN - p, &UTF8StringActualByteCount,
            pos, Entry->Length - 8 - ((pos - Entry->Text) << 1));
        p += UTF8StringActualByteCount;

        NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock,
            buffer, p - buffer, NULL, NULL);
        RtlExitUserProcess(1);
    }

    HANDLE hProcess;
    HANDLE hFile[MAX_DLL];
    DEBUG_EVENT DebugEvent;
    HANDLE hThread[MAX_THREAD];
    PROCESS_INFORMATION processInfo;
    LPVOID BaseOfDll[MAX_DLL] = {};
    STARTUPINFOW startupInfo = {sizeof(startupInfo)};

    if (pCmdLine + len != ptr) *ptr = ' ';

    if (!dwarf)
    {
        // The system closes the handles to the process and thread
        // when the debugger calls the ContinueDebugEvent function
        CreateProcessW(ApplicationName, pNext, NULL, NULL, FALSE,
            start ? CreationFlags | DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE
                  : CreationFlags | DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_PROCESS_GROUP,
            ProcessParameters->Environment, ProcessParameters->CurrentDirectory.DosPath.Buffer, &startupInfo, &processInfo);

        NtClose(processInfo.hThread);
        NtClose(processInfo.hProcess);
        WaitForDebugEvent(&DebugEvent, INFINITE);
        hFile[0] = DebugEvent.u.CreateProcessInfo.hFile;
        BaseOfDll[0] = DebugEvent.u.CreateProcessInfo.lpBaseOfImage;
        hProcess = DebugEvent.u.CreateProcessInfo.hProcess;
        hThread[0] = DebugEvent.u.CreateProcessInfo.hThread;
    } else
    {
        // The system closes the handles to the process and thread
        // when the debugger calls the DebugActiveProcessStop function
        CreateProcessW(ApplicationName, pNext, NULL, NULL, FALSE,
            start ? CreationFlags | CREATE_SUSPENDED | CREATE_NEW_CONSOLE
                  : CreationFlags | CREATE_SUSPENDED | CREATE_NEW_PROCESS_GROUP,
            ProcessParameters->Environment, ProcessParameters->CurrentDirectory.DosPath.Buffer, &startupInfo, &processInfo);

        DebugActiveProcess(processInfo.dwProcessId);
        NtResumeProcess(processInfo.hProcess);
        WaitForDebugEvent(&DebugEvent, INFINITE);
        NtClose(DebugEvent.u.CreateProcessInfo.hFile);
        hThread[0] = processInfo.hThread;
        hProcess = processInfo.hProcess;
    }

    if (verbose >= 2)
    {
        memcpy(buffer, CREATE_PROCESS, sizeof(CREATE_PROCESS));
        p = _ultoa10(DebugEvent.dwProcessId, buffer + sizeof(CREATE_PROCESS));
        *p = 'x';
        p = _ultoa10(DebugEvent.dwThreadId, p + 1);
        *p = '\n';
        NtWriteFile(hStdout, NULL, NULL, NULL,
            &IoStatusBlock, buffer, p - buffer + 1, NULL, NULL);
    }

    // DBG_EXCEPTION_NOT_HANDLED
    // https://learn.microsoft.com/windows/win32/api/debugapi/nf-debugapi-continuedebugevent
    ContinueDebugEvent(DebugEvent.dwProcessId, DebugEvent.dwThreadId, DBG_CONTINUE);

    // x86 process / MinGW64 G++ trigger
    // more than one breakpoints at start-up
    if ((bx64win && dwarf) || !bx64win) --firstbreak;

    unsigned char i;
    DWORD dwThreadId[MAX_THREAD] = {};

    dwThreadId[0] = DebugEvent.dwThreadId;

    while (TRUE)
    {
        WaitForDebugEvent(&DebugEvent, INFINITE);

        // https://learn.microsoft.com/windows/win32/debug/debugging-events
        switch (DebugEvent.dwDebugEventCode)
        {

            case LOAD_DLL_DEBUG_EVENT:
                if (verbose >= 2)
                {
                    wchar_t Tmp[WBUFLEN];

                    memcpy(buffer, LOAD_DLL, sizeof(LOAD_DLL));
                    len = GetFinalPathNameByHandleW(DebugEvent.u.LoadDll.hFile,
                        Tmp, WBUFLEN, FILE_NAME_OPENED);
                    RtlUnicodeToUTF8N(buffer + sizeof(LOAD_DLL),
                        BUFLEN - sizeof(LOAD_DLL), &UTF8StringActualByteCount,
                        Tmp + 4, (len << 1) - 8); /* Skip \\?\ */
                    buffer[UTF8StringActualByteCount + sizeof(LOAD_DLL)] = '\n';

                    NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock, buffer,
                        UTF8StringActualByteCount + sizeof(LOAD_DLL) + 1, NULL, NULL);
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
                        wchar_t Tmp[WBUFLEN];

                        memcpy(buffer, UNLOAD_DLL, sizeof(UNLOAD_DLL));
                        len = GetFinalPathNameByHandleW(hFile[i],
                            Tmp, WBUFLEN, FILE_NAME_OPENED);
                        RtlUnicodeToUTF8N(buffer + sizeof(UNLOAD_DLL),
                            BUFLEN - sizeof(UNLOAD_DLL), &UTF8StringActualByteCount,
                            Tmp + 4, (len << 1) - 8); /* Skip \\?\ */
                        buffer[UTF8StringActualByteCount + sizeof(UNLOAD_DLL)] = '\n';

                        NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock, buffer,
                            UTF8StringActualByteCount + sizeof(UNLOAD_DLL) + 1, NULL, NULL);
                    }

                    NtClose(hFile[i]);
                    BaseOfDll[i] = 0;
                    break;
                }

                break;

            case CREATE_THREAD_DEBUG_EVENT:
                if (verbose >= 2)
                {
                    memcpy(buffer, CREATE_THREAD, sizeof(CREATE_THREAD));
                    p = _ultoa10(DebugEvent.dwProcessId, buffer + sizeof(CREATE_THREAD));
                    *p = 'x';
                    p = _ultoa10(DebugEvent.dwThreadId, p + 1);
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
                    memcpy(buffer, EXIT_THREAD, sizeof(EXIT_THREAD));
                    p = _ultoa10(DebugEvent.dwProcessId, buffer + sizeof(EXIT_THREAD));
                    *p = 'x';
                    p = _ultoa10(DebugEvent.dwThreadId, p + 1);
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
                    memcpy(buffer, EXIT_PROCESS, sizeof(EXIT_PROCESS));
                    p = _ultoa10(DebugEvent.dwProcessId, buffer + sizeof(EXIT_PROCESS));
                    *p = 'x';
                    p = _ultoa10(DebugEvent.dwThreadId, p + 1);
                    *p = '\n';
                    NtWriteFile(hStdout, NULL, NULL, NULL,
                        &IoStatusBlock, buffer, p - buffer + 1, NULL, NULL);
                }

                // Cleanup
                if (dwarf)
                {
                    NtClose(hProcess);
                    NtClose(hThread[0]);
                }

                for (i = 0; i < MAX_DLL; ++i)
                    if (BaseOfDll[i]) NtClose(hFile[i]);

                if (timeout)
                {
                    HANDLE hStdin = ProcessParameters->StandardInput;
                    WaitForInputOrTimeout(hStdin, hStdout, timeout,
                        GetFileType(hStdin) == FILE_TYPE_CHAR);
                }

                ContinueDebugEvent(DebugEvent.dwProcessId, DebugEvent.dwThreadId, DBG_CONTINUE);
                RtlExitUserProcess(0);

            [[fallthrough]];
            case OUTPUT_DEBUG_STRING_EVENT:
                if (verbose >= 2)
                {
                    memcpy(buffer, OUTPUT_DEBUG, sizeof(OUTPUT_DEBUG));
                    p = _ultoa10(DebugEvent.dwProcessId, buffer + sizeof(OUTPUT_DEBUG));
                    *p = 'x';
                    p = _ultoa10(DebugEvent.dwThreadId, p + 1);
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
                    DebugEvent.u.Exception.ExceptionRecord.ExceptionCode == 0x4000001F) && // STATUS_WX86_BREAKPOINT
                    ((breakpoint == FALSE) || (breakpoint == TRUE && ++firstbreak <= 1)))
                    break;

                // Check if not first-chance
                if (!DebugEvent.u.Exception.dwFirstChance)
                {
                    NtTerminateProcess(hProcess, DebugEvent.u.Exception.ExceptionRecord.ExceptionCode);
                    ContinueDebugEvent(DebugEvent.dwProcessId, DebugEvent.dwThreadId, 0x80010001L);
                    continue;
                }

                // Find thread where exception occured
                for (i = 0; i < MAX_THREAD; ++i) if (DebugEvent.dwThreadId == dwThreadId[i])
                    break;

                memcpy(buffer, THREAD_NUMBER, sizeof(THREAD_NUMBER));
                if (DebugEvent.dwThreadId == dwThreadId[i])
                {
                    buffer[sizeof(THREAD_NUMBER)] = '0' + (i + 1) / 10;
                    buffer[sizeof(THREAD_NUMBER) + 1] = '0' + (i + 1) % 10;
                    p = buffer + sizeof(THREAD_NUMBER) + 2;
                } else p = _ultoa10(DebugEvent.dwThreadId, buffer + sizeof(THREAD_NUMBER) - 1);

                wchar_t Tmp[WBUFLEN];

                memcpy(p, THREAD_TRIGGERD, sizeof(THREAD_TRIGGERD));
                p += sizeof(THREAD_TRIGGERD);
                p = _ultoa16(DebugEvent.u.Exception.ExceptionRecord.ExceptionCode, p);

                if (Console)
                {
                    memcpy(p, CONSOLE_NRED_FORMAT, sizeof(CONSOLE_NRED_FORMAT));
                    p += sizeof(CONSOLE_NRED_FORMAT);
                } else *p++ = '\n';

                LDR_DATA_TABLE_ENTRY *NTDLL;
                MESSAGE_RESOURCE_ENTRY *Entry;

                LdrFindEntryForAddress(LdrFindEntryForAddress, &NTDLL);
                FindModuleMessage(NTDLL->DllBase, DebugEvent.u.Exception.ExceptionRecord.ExceptionCode,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &Entry);
                // Convert error message to UTF-8
                RtlUnicodeToUTF8N(p, buffer + BUFLEN - p,
                    &UTF8StringActualByteCount, Entry->Text, Entry->Length - 8);
                p += UTF8StringActualByteCount;

                if (Console)
                {
                    memcpy(p, CONSOLE_DEFAULT_FORMAT, sizeof(CONSOLE_DEFAULT_FORMAT));
                    p += sizeof(CONSOLE_DEFAULT_FORMAT);
                }


                // Check if critical exception
                if (!(DebugEvent.u.Exception.ExceptionRecord.ExceptionCode & EXCEPTION_NONCONTINUABLE) &&
                    // https://learn.microsoft.com/visualstudio/debugger/tips-for-debugging-threads?view=vs-2022&tabs=csharp#set-a-thread-name-by-throwing-an-exception
                    (DebugEvent.u.Exception.ExceptionRecord.ExceptionCode == 0x406D1388 || // MS_VC_EXCEPTION
                    DebugEvent.u.Exception.ExceptionRecord.ExceptionCode == 0xE06D7363 || // STATUS_CPP_EH_EXCEPTION
                    DebugEvent.u.Exception.ExceptionRecord.ExceptionCode == 0xE0434f4D)) // STATUS_CLR_EXCEPTION
                    ContinueDebugEvent(DebugEvent.dwProcessId, DebugEvent.dwThreadId, DBG_CONTINUE);
                else if (dwarf && RtlDosSearchPath_U(PATH, GDB_EXE, NULL,
                    sizeof(ApplicationName), ApplicationName, NULL))
                { // Check if executable exists
                    NtWriteFile(hStdout, NULL, NULL, NULL,
                        &IoStatusBlock, buffer, p - buffer, NULL, NULL);
                    NtSuspendProcess(hProcess);
                    ContinueDebugEvent(DebugEvent.dwProcessId, DebugEvent.dwThreadId, DBG_REPLY_LATER);
                    DebugActiveProcessStop(DebugEvent.dwProcessId);

                    HANDLE hTemp;
                    UNICODE_STRING String;
                    LARGE_INTEGER ByteOffset;
                    OBJECT_ATTRIBUTES ObjectAttributes;
                    wchar_t CommandLine[(sizeof(GDB_COMMAND_LINE) >> 1) + MAX_PATH];

                    memcpy(CommandLine, GDB_COMMAND_LINE, sizeof(GDB_COMMAND_LINE));
                    RtlQueryEnvironmentVariable(ProcessParameters->Environment, TMPENV,
                        sizeof(TMPENV) >> 1, &CommandLine[(sizeof(GDB_COMMAND_LINE) >> 1)],
                        (sizeof(CommandLine) >> 1) - (sizeof(GDB_COMMAND_LINE) >> 1), &temp);

                    // Ensure correct DOS path
                    if (CommandLine[(sizeof(GDB_COMMAND_LINE) >> 1) - 1 + temp] != '\\')
                    {
                        CommandLine[(sizeof(GDB_COMMAND_LINE) >> 1) + temp] = '\\';
                        ++temp;
                    }

                    memcpy(&CommandLine[(sizeof(GDB_COMMAND_LINE) >> 1) + temp], W64DBG, sizeof(W64DBG));
                    String.Length = 8 + sizeof(W64DBG) + (temp << 1);
                    String.Buffer = &CommandLine[(sizeof(GDB_COMMAND_LINE) >> 1) - 4];
                    InitializeObjectAttributes(&ObjectAttributes,
                        &String, OBJ_CASE_INSENSITIVE, NULL, NULL);
                    NtCreateFile(&hTemp, FILE_WRITE_DATA | SYNCHRONIZE,
                        &ObjectAttributes, &IoStatusBlock, NULL, FILE_ATTRIBUTE_NOT_CONTENT_INDEXED,
                        0, FILE_OPEN_IF, FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

                    // First time run
                    if (IoStatusBlock.Information == 2) // FILE_CREATED
                        NtWriteFile(hTemp, NULL, NULL, NULL, &IoStatusBlock,
                            GDB_DEFAULT, sizeof(GDB_DEFAULT), NULL, NULL);

                    p = _ultoa10(DebugEvent.dwProcessId, buffer);

                    if (dwarf == MINGW_NOKEEP)
                    {
                        memcpy(p, GDB_PRESERVE, sizeof(GDB_PRESERVE));
                        p += sizeof(GDB_PRESERVE);
                    } else
                    {
                        memcpy(p, GDB_STYLE, sizeof(GDB_STYLE));
                        p += sizeof(GDB_STYLE);
                    }

                    if (verbose >= 1)
                    {
                        memcpy(p, GDB_FRAME_ARG, sizeof(GDB_FRAME_ARG));
                        p += sizeof(GDB_FRAME_ARG);
                    }

                    memcpy(p, GDB_CONTINUE, sizeof(GDB_CONTINUE));
                    p += sizeof(GDB_CONTINUE);

                    if (verbose >= 1)
                    {
                        *p++ = ' ';
                        *p++ = 'f';
                    }

                    ByteOffset.QuadPart = sizeof(GDB_DEFAULT);
                    NtWriteFile(hTemp, NULL, NULL, NULL, &IoStatusBlock,
                        buffer, p - buffer, &ByteOffset, NULL);

                    ByteOffset.QuadPart +=  p - buffer;
                    NtSetInformationFile(hTemp, &IoStatusBlock, &ByteOffset,
                        sizeof(LARGE_INTEGER), 20); // FileEndOfFileInformation
                    NtClose(hTemp);

                    if (dwarf == MINGW_KEEP && !timeout)
                        memcpy(&CommandLine[(sizeof(GDB_COMMAND_LINE) >> 1) + temp + 6], GDB_BATCH, sizeof(GDB_BATCH));

                    DWORD_PTR ProcDbgPt;
                    LARGE_INTEGER DelayInterval;

                    CreateProcessW(ApplicationName, CommandLine, NULL, NULL,
                        FALSE, CreationFlags, ProcessParameters->Environment,
                        ProcessParameters->CurrentDirectory.DosPath.Buffer, &startupInfo, &processInfo);
                    NtClose(processInfo.hThread);
                    NtClose(hThread[0]);

                    for (i = 0; i < MAX_DLL; ++i)
                        if (BaseOfDll[i]) NtClose(hFile[i]);

                    // Convert seconds to 100-nanosecond units
                    // (negative for relative time)
                    DelayInterval.QuadPart = -(LATENCY * 10000LL);

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
                        hStdin = ProcessParameters->StandardInput;
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
                            &IoStatusBlock, GDB_QUIT, sizeof(GDB_QUIT), NULL, NULL);
                    }

                    NtWaitForMultipleObjects(1, &processInfo.hProcess, WaitAll, FALSE, NULL);
                    NtClose(processInfo.hProcess);

                    if (verbose >= 2)
                    {
                        memcpy(buffer, EXIT_PROCESS, sizeof(EXIT_PROCESS));
                        p = _ultoa10(DebugEvent.dwProcessId, buffer + sizeof(EXIT_PROCESS));
                        *p = 'x';
                        p = _ultoa10(DebugEvent.dwThreadId, p + 1);
                        *p = '\n';
                        NtWriteFile(hStdout, NULL, NULL, NULL,
                            &IoStatusBlock, buffer, p - buffer + 1, NULL, NULL);
                    }

                    if (timeout) WaitForInputOrTimeout(hStdin,
                        hStdout, timeout, StdinConsole);

                    RtlExitUserProcess(0);
                } else ContinueDebugEvent(DebugEvent.dwProcessId, DebugEvent.dwThreadId, 0x80010001L);

                if (DebugEvent.dwThreadId != dwThreadId[i])
                {
                    NtWriteFile(hStdout, NULL, NULL, NULL,
                        &IoStatusBlock, buffer, p - buffer, NULL, NULL);
                    continue;
                }

                DWORD MachineType;
                STACKFRAME64 StackFrame;
                FILE_STANDARD_INFORMATION FileInfo;

                SymSetOptions(bx64win ? SymOptions : SymOptions | SYMOPT_INCLUDE_32BIT_MODULES);
                SymInitializeW(hProcess, NULL, FALSE);

                for (unsigned char j = 0; j < MAX_DLL; ++j) if (BaseOfDll[j])
                {
                    NtQueryInformationFile(hFile[j], &IoStatusBlock,
                        &FileInfo, sizeof(FileInfo), 5); // FileStandardInformation
                    SymLoadModuleExW(hProcess,
                        hFile[j],
                        NULL,
                        NULL,
                        (DWORD64) BaseOfDll[j],
                        FileInfo.EndOfFile.LowPart,
                        NULL,
                        0);
                }

                CONTEXT Context;

                if (bx64win)
                {
                    Context.ContextFlags = CONTEXT_ALL;
                    NtGetContextThread(hThread[i], &Context);
                    MachineType = IMAGE_FILE_MACHINE_AMD64;
                    StackFrame.AddrPC.Offset = Context.Rip;
                    StackFrame.AddrStack.Offset = Context.Rsp;
                    StackFrame.AddrFrame.Offset = Context.Rbp;
                } else
                {
                    ((PWOW64_CONTEXT) &Context)->ContextFlags = WOW64_CONTEXT_ALL;
                    NtQueryInformationThread(hThread[i], 29, // ThreadWow64Context
                        &Context, sizeof(WOW64_CONTEXT), NULL);
                    // Find no replacable NTAPI
                    MachineType = IMAGE_FILE_MACHINE_I386;
                    StackFrame.AddrPC.Offset = ((PWOW64_CONTEXT) &Context)->Eip;
                    StackFrame.AddrStack.Offset = ((PWOW64_CONTEXT) &Context)->Esp;
                    StackFrame.AddrFrame.Offset = ((PWOW64_CONTEXT) &Context)->Ebp;
                }

                char Success;
                DWORD Displacement;
                char Symbol[BUFLEN];
                unsigned char count;
                PSYMBOL_INFOW pSymbol;
                IMAGEHLP_LINEW64 Line;
                USERCONTEXT UserContext;

                pSymbol = (PSYMBOL_INFOW) Symbol;
                Line.SizeOfStruct = sizeof(Line);
                StackFrame.AddrPC.Mode = AddrModeFlat;
                StackFrame.AddrStack.Mode = AddrModeFlat;
                StackFrame.AddrFrame.Mode = AddrModeFlat;
                pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
                pSymbol->MaxNameLen = (BUFLEN - sizeof(SYMBOL_INFO)) >> 1;

                count = 0;
                UserContext.hProcess = hProcess;
                UserContext.pContext = &Context;
                UserContext.pBase = &StackFrame.AddrFrame.Offset;
                UserContext.bx64win = bx64win;
                UserContext.Console = Console;

                while (TRUE)
                {
                    if (!StackWalk64(MachineType, hProcess, hThread[i],
                        &StackFrame, &Context, NULL, NULL, NULL, NULL))
                        break;

                    if (!SymFromAddr(hProcess, StackFrame.AddrPC.Offset, NULL, pSymbol))
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
                        memcpy(p, CONSOLE_BLUE_FORMAT, sizeof(CONSOLE_BLUE_FORMAT));
                        p += sizeof(CONSOLE_BLUE_FORMAT);
                    }

                    p = _ui64toa16(StackFrame.AddrPC.Offset, p, bx64win);

                    if (Console)
                    {
                        memcpy(p, EXCEPTION_IN, sizeof(EXCEPTION_IN));
                        p += sizeof(EXCEPTION_IN);
                    } else
                    {
                        memcpy(p, EXCEPTION_IN + 3, 4);
                        p += 4;
                    }

                    memcpy(p, pSymbol->Name, pSymbol->NameLen);
                    p += pSymbol->NameLen;

                    if (Console)
                    {
                        memcpy(p, CONSOLE_DEFAULT_FORMAT,
                            sizeof(CONSOLE_DEFAULT_FORMAT));
                        p += sizeof(CONSOLE_DEFAULT_FORMAT);
                    }

                    *p++ = ' ';
                    *p++ = '(';

                    if (SymSetScopeFromIndex(hProcess, pSymbol->ModBase, pSymbol->Index))
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

                    if (SymGetLineFromAddrW64(hProcess,
                        StackFrame.AddrPC.Offset, &Displacement, &Line))
                    {
                        memcpy(p, EXCEPTION_AT, sizeof(EXCEPTION_AT));
                        temp = wcslen(Line.FileName);
                        p += sizeof(EXCEPTION_AT);

                        if (Console)
                        {
                            memcpy(p, CONSOLE_GREEN_FORMAT, sizeof(CONSOLE_GREEN_FORMAT));
                            p += sizeof(CONSOLE_GREEN_FORMAT);
                        }

                        // Skip %dir%/
                        if (temp > wDirLen && !RtlCompareUnicodeStrings(Line.FileName,
                                wDirLen, PATH, wDirLen, TRUE))
                            p = FormatFileLine(Line.FileName + wDirLen + 1,
                                Line.LineNumber, temp - wDirLen - 1, p, Console);
                        else p = FormatFileLine(Line.FileName,
                            Line.LineNumber, temp, p, Console);
                        if (verbose >= 1) p = FormatSourceCode(Line.FileName, Line.LineNumber, temp, buffer, p, verbose);
                    } else
                    {
                        memcpy(p, EXCEPTION_FROM, sizeof(EXCEPTION_FROM));
                        p += sizeof(EXCEPTION_FROM);

                        if (Console)
                        {
                            memcpy(p, CONSOLE_GREEN_FORMAT, sizeof(CONSOLE_GREEN_FORMAT));
                            p += sizeof(CONSOLE_GREEN_FORMAT);
                        }

                        len = GetModuleFileNameExW(hProcess,
                            (HMODULE) SymGetModuleBase64(hProcess,
                                StackFrame.AddrPC.Offset), Tmp, WBUFLEN);
                        RtlUnicodeToUTF8N(p, buffer + BUFLEN - p,
                            &UTF8StringActualByteCount, Tmp, len << 1);
                        p += UTF8StringActualByteCount;

                        if (Console)
                        {
                            memcpy(p, CONSOLE_DEFAULT_FORMAT,
                                sizeof(CONSOLE_DEFAULT_FORMAT));
                            p += sizeof(CONSOLE_DEFAULT_FORMAT);
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

                /*
                for (i = 0; i < MAX_DLL; ++i) if (BaseOfDll[i])
                    SymUnloadModule64(hProcess,
                        (DWORD64) BaseOfDll[i]);
                */

                SymCleanup(hProcess);

                continue;

            case RIP_EVENT:
                if (verbose >= 2)
                {
                    MESSAGE_RESOURCE_ENTRY *Entry;

                    memcpy(buffer, RIP, sizeof(RIP));
                    p = _ultoa10(DebugEvent.dwProcessId, buffer + sizeof(RIP));
                    *p = 'x';
                    p = _ultoa10(DebugEvent.dwThreadId, p + 1);
                    *p++ = '\n';

                    FindSystemMessage(DebugEvent.u.RipInfo.dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &Entry);
                    // Convert error message to UTF-8
                    RtlUnicodeToUTF8N(p, buffer + BUFLEN - p,
                        &UTF8StringActualByteCount, Entry->Text, Entry->Length - 8);
                    p += UTF8StringActualByteCount;

                    if (DebugEvent.u.RipInfo.dwType == 1)
                    {
                        memcpy(p, _SLE_ERROR, sizeof(_SLE_ERROR));
                        p += sizeof(_SLE_ERROR);
                    } else if (DebugEvent.u.RipInfo.dwType == 2)
                    {
                        memcpy(p, _SLE_MINORERROR, sizeof(_SLE_MINORERROR));
                        p += sizeof(_SLE_MINORERROR);
                    } else if (DebugEvent.u.RipInfo.dwType == 3)
                    {
                        memcpy(p, _SLE_WARNING, sizeof(_SLE_WARNING));
                        p += sizeof(_SLE_WARNING);
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