/*
    Copyright (c) 2024-2025 Xuan Tan. All rights reserved.
    Licensed under the BSD-3-Clause.
*/

#pragma once

/*
    Retrieves a localized system message from ntdll.dll.
    Leverages direct access to the second loaded module in the PEB loader data.
*/

static
__forceinline
NTSTATUS
LookupNtdllMessage(
    PPEB_LDR_DATA            Ldr,
    DWORD                    dwMessageId,
    DWORD                    dwLanguageId,
    PMESSAGE_RESOURCE_ENTRY *Entry)
{
    return RtlFindMessage(
        ((PLDR_DATA_TABLE_ENTRY) ((PLIST_ENTRY) Ldr->Reserved2[1])->Flink)->DllBase,
        (ULONG)(ULONG_PTR) RT_MESSAGETABLE, dwLanguageId, dwMessageId, Entry);
}

/*
    Retrieves a localized system message from KernelBase.dll.
    Leverages direct access to the fourth loaded module in the PEB loader data.
*/

static
__forceinline
NTSTATUS
LookupSystemMessage(
    PPEB_LDR_DATA            Ldr,
    DWORD                    dwMessageId,
    DWORD                    dwLanguageId,
    PMESSAGE_RESOURCE_ENTRY *Entry)
{
    return RtlFindMessage(
        ((PLDR_DATA_TABLE_ENTRY) ((PLIST_ENTRY) Ldr->Reserved2[1])->Flink->Flink->Flink)->DllBase,
        (ULONG)(ULONG_PTR) RT_MESSAGETABLE, dwLanguageId, dwMessageId, Entry);
}

/*
    Formats a debug event message into a buffer.
    Output format: "<EventName><ProcessID>x<ThreadID>\n"
*/

static
FORCEINLINE
ULONG
FormatDebugEvent(
    LPDEBUG_EVENT lpDebugEvent,
    LPCSTR        szDebugEventName,
    SIZE_T        DebugEventNameLength,
    LPSTR         Buffer)
{
    char *p;
    
    memcpy(Buffer, szDebugEventName, DebugEventNameLength);
    p = jeaiii::to_ascii_chars(Buffer + DebugEventNameLength, lpDebugEvent->dwProcessId);
    *p = 'x';
    p = jeaiii::to_ascii_chars(p + 1, lpDebugEvent->dwThreadId);
    *p = '\n';

    return (ULONG)(p - Buffer + 1);
}

/*
    Formats a module-related debug event message into a buffer.
    Output format: "<EventName><ModulePath>\n"
*/

static
FORCEINLINE
ULONG
FormatModuleEvent(
    LPDEBUG_EVENT lpDebugEvent,
    LPCSTR        szDebugEventName,
    SIZE_T        DebugEventNameLength,
    LPSTR         Buffer)
{
    DWORD Length;
    ULONG ActualByteCount;
    wchar_t Temp[MAX_PATH];

    memcpy(Buffer, szDebugEventName, DebugEventNameLength);
    Length = GetFinalPathNameByHandleW(lpDebugEvent->u.LoadDll.hFile, Temp, MAX_PATH, FILE_NAME_OPENED);
    RtlUnicodeToUTF8N(Buffer + DebugEventNameLength, MAX_PATH,
        &ActualByteCount, Temp + 4, (Length << 1) - 8); /* Skip \\?\ */
    Buffer[ActualByteCount + DebugEventNameLength] = '\n';

    return (ULONG)(ActualByteCount + DebugEventNameLength + 1);
}

/*
    Formats a RIP (Raise an Exception) debug event message into a buffer.
    Output format: "<ErrorMessage><ErrorType>\n"
*/

static
FORCEINLINE
ULONG
FormatRIPEvent(
    LPDEBUG_EVENT lpDebugEvent,
    PPEB_LDR_DATA Ldr,
    LPSTR         Buffer,
    ULONG         BufLen)
{
    char *p;
    ULONG ActualByteCount;
    PMESSAGE_RESOURCE_ENTRY Entry;

    LookupSystemMessage(Ldr, lpDebugEvent->u.RipInfo.dwError, LANG_USER_DEFAULT, &Entry);
    RtlUnicodeToUTF8N(Buffer, BufLen, &ActualByteCount, (PCWSTR) Entry->Text, Entry->Length - 8);
    p = Buffer + ActualByteCount;

    if (lpDebugEvent->u.RipInfo.dwType == 1)
    {
        memcpy(p, _SLE_ERROR, strlen(_SLE_ERROR));
        p += strlen(_SLE_ERROR);
    } else if (lpDebugEvent->u.RipInfo.dwType == 2)
    {
        memcpy(p, _SLE_MINORERROR, strlen(_SLE_MINORERROR));
        p += strlen(_SLE_MINORERROR);
    } else if (lpDebugEvent->u.RipInfo.dwType == 3)
    {
        memcpy(p, _SLE_WARNING, strlen(_SLE_WARNING));
        p += strlen(_SLE_WARNING);
    }

    if (lpDebugEvent->u.RipInfo.dwType) *p++ = '.';
    *p = '\n';

    return p - Buffer + 1;
}

static __forceinline LPSTR FormatFileLine(LPWSTR FileName, DWORD LineNumber, ULONG FileLength, ULONG BufLength, LPSTR p, BOOL Console)
{
    ULONG ActualByteCount;

    // Convert file name from Unicode to UTF-8
    RtlUnicodeToUTF8N(p, BufLength,
        &ActualByteCount, FileName, FileLength);
    p += ActualByteCount;

    // Optionally apply color formatting
    if (Console)
    {
        memcpy(p, CONSOLE_DEFAULT_FORMAT,
            strlen(CONSOLE_DEFAULT_FORMAT));
        p += strlen(CONSOLE_DEFAULT_FORMAT);
    }

    *p = ':'; // Add a colon separator
    p = jeaiii::to_ascii_chars(p + 1, LineNumber);
    *p = '\n'; // Terminate with a newline

    return p + 1;
}

static __forceinline LPSTR FormatSourceCode(PPEB_LDR_DATA Ldr, LPWSTR FileName, DWORD LineNumber, size_t FileLength, ULONG BufLength, LPSTR p)
{
    // Prepend Object Manager namespace to the file name
    memcpy(FileName - OBJECT_MANAGER_NAMESPACE_WLEN,
        OBJECT_MANAGER_NAMESPACE, OBJECT_MANAGER_NAMESPACE_LEN);

    UNICODE_STRING String;
    String.Length = FileLength + OBJECT_MANAGER_NAMESPACE_LEN;
    String.Buffer = FileName - OBJECT_MANAGER_NAMESPACE_WLEN;

    HANDLE hFile;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes = {sizeof(OBJECT_ATTRIBUTES), NULL, &String, OBJ_CASE_INSENSITIVE, NULL, NULL};

    // Open the file with necessary permissions
    NtOpenFile(&hFile, FILE_READ_DATA | SYNCHRONIZE, &ObjectAttributes,
        &IoStatusBlock, 0, FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT);

    if (IoStatusBlock.Information == FILE_OPENED)
    {
        char *ptr;
        char buffer[PAGESIZE];
        DWORD line = 1;

        while (TRUE)
        { // Read file content in chunks
            if (NtReadFile(hFile, NULL, NULL, NULL, &IoStatusBlock,
                buffer, sizeof(buffer), NULL, NULL) != 0) break; // STATUS_SUCCESS

            ptr = buffer;

            while ((ptr = (char*) memchr(ptr, '\n', buffer + IoStatusBlock.Information - ptr) + 1) > (char*) 1)
            { // Locate and process line breaks
                if (++line == LineNumber)
                {
                    char *_ptr;
                    size_t temp;

                    // Format line number and spacing
                    p = jeaiii::to_ascii_chars(p, line);
                    memset(p, ' ', 6);
                    p += 6;
                    // Extract content for the target line
                    _ptr = (char*) memchr(ptr, '\n', buffer + IoStatusBlock.Information - ptr);

                    if (_ptr) temp = _ptr - ptr;
                    else
                    { // Continue reading if line spans multiple chunks
                        temp = buffer + IoStatusBlock.Information - ptr;
                        memcpy(p, ptr, temp);
                        p += temp;
                        if (NtReadFile(hFile, NULL, NULL, NULL, &IoStatusBlock, buffer,
                            sizeof(buffer), NULL, NULL) != 0) break; // STATUS_SUCCESS
                        ptr = buffer;
                        _ptr = (char*) memchr(buffer, '\n', IoStatusBlock.Information);
                        if (_ptr) temp = buffer + IoStatusBlock.Information - _ptr;
                        else temp = IoStatusBlock.Information;
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
    } else
    {  // Handle file not found error in verbose mode
        PMESSAGE_RESOURCE_ENTRY Entry;
        ULONG ActualByteCount;

        LookupSystemMessage(Ldr, ERROR_FILE_NOT_FOUND, LANG_USER_DEFAULT, &Entry);
        // Convert error message to UTF-8
        RtlUnicodeToUTF8N(p, BufLength, &ActualByteCount,
            (PCWSTR) Entry->Text, Entry->Length - 8);
        p += ActualByteCount;
    }

    return p;
}