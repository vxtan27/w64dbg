// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

#include "ntdll.h"

//
//  Format a debug event message into a buffer
//  Output format: "<EventName><ProcessID>x<ThreadID>\n
//

DWORD FormatDebugEvent(PDBGUI_WAIT_STATE_CHANGE pStateChange, PCSTR szDebugEventName, SIZE_T DebugEventNameLength, PCH Buffer) {
    memcpy(Buffer, szDebugEventName, DebugEventNameLength);
    char *p = conversion::dec::from_int(Buffer + DebugEventNameLength,
        DbgGetProcessId(pStateChange));
    *p = 'x';
    p = conversion::dec::from_int(p + 1, DbgGetThreadId(pStateChange));
    *p = '\n';

    return p - Buffer + 1;
}

//
//  Format a module-related debug event message into a buffer
//  Output format: "<EventName><ModulePath>\n"
//

DWORD FormatDebugModule(HANDLE hModule, PCSTR szDebugEventName, SIZE_T DebugEventNameLength, PCH Buffer) {
    SIZE_T Length;
    ULONG ActualByteCount;
    WCHAR Target[MAX_PATH];
    WCHAR Drive[] = L"A:";
    CHAR Temp[sizeof(OBJECT_NAME_INFORMATION) + MAX_PATH * sizeof(WCHAR)];
    POBJECT_NAME_INFORMATION NameInfo = (POBJECT_NAME_INFORMATION) &Temp;

    memcpy(Buffer, szDebugEventName, DebugEventNameLength);
    NtQueryObject(hModule, ObjectNameInformation, NameInfo, sizeof(Temp), NULL);

    for (WCHAR Letter = L'A'; Letter <= L'L'; ++Letter) {
        Drive[0] = Letter;
        if (QueryDosDeviceW(Drive, Target, MAX_PATH)) {
            Length = wcslen(Target);
            if (memcmp(NameInfo->Name.Buffer, Target, Length << 1) == 0) {
                Buffer[DebugEventNameLength] = Letter;
                Buffer[DebugEventNameLength + 1] = ':';
                RtlUnicodeToUTF8N(Buffer + DebugEventNameLength + 2,
                    MAX_PATH, &ActualByteCount, NameInfo->Name.Buffer + Length,
                    NameInfo->Name.Length - (Length << 1));
                Buffer[ActualByteCount + DebugEventNameLength + 2] = '\n';

                return ActualByteCount + DebugEventNameLength + 2 + 1;
            }
        }
    }

    std::unreachable();
}

#define _SLE_ERROR       "Invalid data was passed to the function that failed. This caused the application to fail"
#define _SLE_MINORERROR  "Invalid data was passed to the function, but the error probably will not cause the application to fail"
#define _SLE_WARNING     "Potentially invalid data was passed to the function, but the function completed processing"

//
//  Format a RIP (Raise an Exception) debug event message into a buffer
//  Output format: "<ErrorMessage><ErrorType>\n"
//

DWORD FormatRIPEvent(PEXCEPTION_RECORD pExceptionRecord, PCH Buffer, ULONG BufLen) {
    char *p;
    PMESSAGE_RESOURCE_ENTRY MessageEntry;

    if (NT_SUCCESS(LookupSystemMessage(
        pExceptionRecord->ExceptionFlags, LANG_USER_DEFAULT, &MessageEntry)))
        p = Buffer + ConvertUnicodeToUTF8(GetMessageEntryText(MessageEntry),
            GetMessageEntryLength(MessageEntry), Buffer, BufLen);
    else p = Buffer;

    if (PtrToUlong(pExceptionRecord->ExceptionRecord) == 1) {
        memcpy(p, _SLE_ERROR, strlen(_SLE_ERROR));
        p += strlen(_SLE_ERROR);
    } else if (PtrToUlong(pExceptionRecord->ExceptionRecord) == 2) {
        memcpy(p, _SLE_MINORERROR, strlen(_SLE_MINORERROR));
        p += strlen(_SLE_MINORERROR);
    } else if (PtrToUlong(pExceptionRecord->ExceptionRecord) == 3) {
        memcpy(p, _SLE_WARNING, strlen(_SLE_WARNING));
        p += strlen(_SLE_WARNING);
    }

    if (PtrToUlong(pExceptionRecord->ExceptionRecord)) *p++ = '.';
    *p = '\n';

    return p - Buffer + 1;
}

#define STATUS_APPLICATION_HANG 0xCFFFFFFF
#define STATUS_APPLICATION_HANG_TEXT \
R"({EXCEPTION}
Application hang
The application has stopped responding.
)"

#define STATUS_CPP_EH_EXCEPTION 0xE06D7363
#define STATUS_CPP_EH_EXCEPTION_TEXT \
R"({EXCEPTION}
C++ exception handling exception
An exception occurred during C++ exception handling processing.
)"

#define STATUS_CLR_EXCEPTION 0xE0434f4D
#define STATUS_CLR_EXCEPTION_TEXT \
R"({EXCEPTION}
Common language runtime (CLR) exception
An exception was raised by the Common Language Runtime (CLR).
)"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4701 4703)
#endif

//
//  Format and write a known exception message into the buffer
//  Optionally wrap the message with console color formatting
//

ULONGLONG FormatExceptionEvent(
    NTSTATUS NtStatus,
    DWORD dwLanguageId,
    PCH pBuffer,
    DWORD dwSize,
    BOOL fConsole
) {
    PMESSAGE_RESOURCE_ENTRY MessageEntry;

    // Handle special known statuses or fallback to NTDLL lookup
    if (NtStatus == STATUS_APPLICATION_HANG ||
        NtStatus == STATUS_CPP_EH_EXCEPTION ||
        NtStatus == STATUS_CLR_EXCEPTION ||
        NT_SUCCESS(LookupSystemMessage(RtlNtStatusToDosError(NtStatus), dwLanguageId, &MessageEntry))) {

        PCH lpBuffer = pBuffer;

        if (fConsole) {
            memcpy(lpBuffer, CONSOLE_NRED_FORMAT, strlen(CONSOLE_NRED_FORMAT));
            lpBuffer += strlen(CONSOLE_NRED_FORMAT);
        }

        // Write corresponding message
        if (NtStatus == STATUS_APPLICATION_HANG) {
            memcpy(lpBuffer, STATUS_APPLICATION_HANG_TEXT,
                strlen(STATUS_APPLICATION_HANG_TEXT));
            lpBuffer += strlen(STATUS_APPLICATION_HANG_TEXT);
        } else if (NtStatus == STATUS_CPP_EH_EXCEPTION) {
            memcpy(lpBuffer, STATUS_CPP_EH_EXCEPTION_TEXT,
                strlen(STATUS_CPP_EH_EXCEPTION_TEXT));
            lpBuffer += strlen(STATUS_CPP_EH_EXCEPTION_TEXT);
        } else if (NtStatus == STATUS_CLR_EXCEPTION) {
            memcpy(lpBuffer, STATUS_CLR_EXCEPTION_TEXT,
                strlen(STATUS_CLR_EXCEPTION_TEXT));
            lpBuffer += strlen(STATUS_CLR_EXCEPTION_TEXT);
        } else { // Convert NTDLL message
            lpBuffer += ConvertUnicodeToUTF8(GetMessageEntryText(MessageEntry),
                GetMessageEntryLength(MessageEntry), lpBuffer, dwSize);
        }

        if (fConsole) {
            memcpy(lpBuffer, CONSOLE_DEFAULT_FORMAT, strlen(CONSOLE_DEFAULT_FORMAT));
            lpBuffer += strlen(CONSOLE_DEFAULT_FORMAT);
        }

        return lpBuffer - pBuffer;
    }

    return 0;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

PCH FormatFileLine(PWCH FileName, DWORD LineNumber, ULONG FileLength, ULONG BufLength, PCH p, BOOL Console) {
    // Convert file name from Unicode to UTF-8
    p += ConvertUnicodeToUTF8(FileName, FileLength, p, BufLength);

    // Optionally apply color formatting
    if (Console) {
        memcpy(p, CONSOLE_DEFAULT_FORMAT,
            strlen(CONSOLE_DEFAULT_FORMAT));
        p += strlen(CONSOLE_DEFAULT_FORMAT);
    }

    *p = ':'; // Add a colon separator
    p = conversion::dec::from_int(p + 1, LineNumber);
    *p = '\n'; // Terminate with a newline

    return p + 1;
}

#define OBJECT_MANAGER_NAMESPACE "\\\0?\0?\0\\"
#define OBJECT_MANAGER_NAMESPACE_LEN 8
#define OBJECT_MANAGER_NAMESPACE_WLEN 4

PCH FormatSourceCode(PWCH FileName, DWORD LineNumber, size_t FileLength, ULONG BufLength, PCH p) {
    // Prepend Object Manager namespace to the file name
    PWCH pFileName = (PWCH) _alloca(FileLength + OBJECT_MANAGER_NAMESPACE_LEN);
    memcpy(pFileName, OBJECT_MANAGER_NAMESPACE, OBJECT_MANAGER_NAMESPACE_LEN);
    memcpy(pFileName + OBJECT_MANAGER_NAMESPACE_WLEN, FileName, FileLength);

    UNICODE_STRING String;
    String.Length = FileLength + OBJECT_MANAGER_NAMESPACE_LEN;
    String.Buffer = pFileName;

    HANDLE hFile;
    IO_STATUS_BLOCK IoStatus;
    OBJECT_ATTRIBUTES ObjectAttributes = {
        sizeof(OBJECT_ATTRIBUTES), NULL,
        &String, OBJ_CASE_INSENSITIVE};

    // Open the file with necessary permissions
    NTSTATUS NtStatus = NtOpenFile(&hFile, FILE_READ_DATA | SYNCHRONIZE, &ObjectAttributes,
        &IoStatus, 0, FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT);

    if (NT_SUCCESS(NtStatus)) {
        char *ptr;
        char buffer[PAGE_SIZE];
        DWORD line = 1;

        while (TRUE) { // Read file content in chunks
            if (NtReadFile(hFile, NULL, NULL, NULL,
                &IoStatus, buffer, sizeof(buffer), NULL, NULL)) break;

            ptr = buffer;

            while ((ptr = (char*) memchr(ptr, '\n', buffer + IoStatus.Information - ptr) + 1) > (char*) 1) { // Locate and process line breaks
                if (++line == LineNumber) {
                    char *_ptr;
                    size_t temp;

                    // Format line number and spacing
                    p = conversion::dec::from_int(p, line);
                    memset(p, ' ', 6);
                    p += 6;
                    // Extract content for the target line
                    _ptr = (char*) memchr(ptr, '\n', buffer + IoStatus.Information - ptr);

                    if (_ptr) temp = _ptr - ptr;
                    else { // Continue reading if line spans multiple chunks
                        temp = buffer + IoStatus.Information - ptr;
                        memcpy(p, ptr, temp);
                        p += temp;
                        if (NtReadFile(hFile, NULL, NULL, NULL, &IoStatus,
                            buffer, sizeof(buffer), NULL, NULL)) break;
                        ptr = buffer;
                        _ptr = (char*) memchr(buffer, '\n', IoStatus.Information);
                        if (_ptr) temp = buffer + IoStatus.Information - _ptr;
                        else temp = IoStatus.Information;
                    }

                    ++temp;
                    memcpy(p, ptr, temp);
                    p += temp;
                    break;
                }
            }

            if (line == LineNumber) break;
        }

        NtClose(hFile); // Close the file handle
    } else {
        // The system cannot find the file specified
        PMESSAGE_RESOURCE_ENTRY MessageEntry;

        LookupSystemMessage(RtlNtStatusToDosError(NtStatus), LANG_USER_DEFAULT, &MessageEntry);
        p += ConvertUnicodeToUTF8(GetMessageEntryText(MessageEntry),
            GetMessageEntryLength(MessageEntry), p, BufLength);
    }

    return p;
}