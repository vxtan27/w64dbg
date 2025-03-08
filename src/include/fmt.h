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
    PDBGUI_WAIT_STATE_CHANGE pStateChange,
    LPCSTR        szDebugEventName,
    SIZE_T        DebugEventNameLength,
    LPSTR         Buffer)
{
    char *p;
    
    memcpy(Buffer, szDebugEventName, DebugEventNameLength);
    p = jeaiii::to_ascii_chars(Buffer + DebugEventNameLength,
        HandleToUlong(pStateChange->AppClientId.UniqueProcess));
    *p = 'x';
    p = jeaiii::to_ascii_chars(p + 1, HandleToUlong(pStateChange->AppClientId.UniqueThread));
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
    HANDLE        hModule,
    LPCSTR        szDebugEventName,
    SIZE_T        DebugEventNameLength,
    LPSTR         Buffer)
{
    SIZE_T Length;
    ULONG ActualByteCount;
    WCHAR Target[MAX_PATH];
    WCHAR Drive[3] = L"A:";
    CHAR Temp[sizeof(OBJECT_NAME_INFORMATION) + MAX_PATH * sizeof(WCHAR)];
    POBJECT_NAME_INFORMATION NameInfo = (POBJECT_NAME_INFORMATION) &Temp;

    memcpy(Buffer, szDebugEventName, DebugEventNameLength);
    NtQueryObject(hModule, ObjectNameInformation, NameInfo, sizeof(Temp), NULL);

    for (WCHAR Letter = L'A'; Letter <= L'L'; ++Letter)
    {
        Drive[0] = Letter;
        if (QueryDosDeviceW(Drive, Target, MAX_PATH))
        {
            Length = wcslen(Target);
            if (memcmp(NameInfo->Name.Buffer, Target, Length << 1) == 0)
            {
                Buffer[DebugEventNameLength] = Letter;
                Buffer[DebugEventNameLength + 1] = ':';
            }
        }
    }

    RtlUnicodeToUTF8N(Buffer + DebugEventNameLength + 2, MAX_PATH,
        &ActualByteCount, NameInfo->Name.Buffer + Length, NameInfo->Name.Length - (Length << 1));
    Buffer[ActualByteCount + DebugEventNameLength + 2] = '\n';

    return (ULONG)(ActualByteCount + DebugEventNameLength + 2 + 1);
}

/*
    Formats a RIP (Raise an Exception) debug event message into a buffer.
    Output format: "<ErrorMessage><ErrorType>\n"
*/

static
FORCEINLINE
ULONG
FormatRIPEvent(
    PDBGUI_WAIT_STATE_CHANGE pStateChange,
    PPEB_LDR_DATA            Ldr,
    LPSTR                    Buffer,
    ULONG                    BufLen)
{
    char *p;
    ULONG ActualByteCount;
    PMESSAGE_RESOURCE_ENTRY Entry;

    LookupSystemMessage(Ldr, pStateChange->StateInfo.Exception.ExceptionRecord.ExceptionFlags, LANG_USER_DEFAULT, &Entry);
    RtlUnicodeToUTF8N(Buffer, BufLen, &ActualByteCount, (PCWSTR) Entry->Text, Entry->Length - 8);
    p = Buffer + ActualByteCount;

    if (PtrToUlong(pStateChange->StateInfo.Exception.ExceptionRecord.ExceptionRecord) == 1)
    {
        memcpy(p, _SLE_ERROR, strlen(_SLE_ERROR));
        p += strlen(_SLE_ERROR);
    } else if (PtrToUlong(pStateChange->StateInfo.Exception.ExceptionRecord.ExceptionRecord) == 2)
    {
        memcpy(p, _SLE_MINORERROR, strlen(_SLE_MINORERROR));
        p += strlen(_SLE_MINORERROR);
    } else if (PtrToUlong(pStateChange->StateInfo.Exception.ExceptionRecord.ExceptionRecord) == 3)
    {
        memcpy(p, _SLE_WARNING, strlen(_SLE_WARNING));
        p += strlen(_SLE_WARNING);
    }

    if (PtrToUlong(pStateChange->StateInfo.Exception.ExceptionRecord.ExceptionRecord)) *p++ = '.';
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
            if (NtReadFile(hFile, NULL, NULL, NULL,
                &IoStatusBlock, buffer, sizeof(buffer), NULL, NULL)) break;

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
                        if (NtReadFile(hFile, NULL, NULL, NULL, &IoStatusBlock,
                            buffer, sizeof(buffer), NULL, NULL)) break;
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