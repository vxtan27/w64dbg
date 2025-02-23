/*
    Copyright (c) 2024-2025 Xuan Tan. All rights reserved.
    Licensed under the BSD-3-Clause.
*/

#pragma once

static __forceinline VOID FindNativeMessage(
    PPEB_LDR_DATA Ldr,
    DWORD dwMessageId,
    DWORD dwLanguageId,
    PMESSAGE_RESOURCE_ENTRY *Entry
    )
{
    // Retrieve the base address of the second loaded module (kernel32.dll)
    // Locate the message resource entry
    RtlFindMessage(((PLDR_DATA_TABLE_ENTRY) ((PLIST_ENTRY) Ldr->Reserved2[1])->Flink)->DllBase,
        (ULONG)(ULONG_PTR) RT_MESSAGETABLE, dwLanguageId, dwMessageId, Entry);
}

static __forceinline VOID FindCoreMessage(
    PPEB_LDR_DATA Ldr,
    DWORD dwMessageId,
    DWORD dwLanguageId,
    PMESSAGE_RESOURCE_ENTRY *Entry
    )
{
    // Retrieve the base address of the fifth loaded module (ntdll.dll)
    // Locate the message resource entry
    RtlFindMessage(((PLDR_DATA_TABLE_ENTRY) ((PLIST_ENTRY) Ldr->Reserved2[1])->Flink->Flink->Flink)->DllBase,
        (ULONG)(ULONG_PTR) RT_MESSAGETABLE, dwLanguageId, dwMessageId, Entry);
}

static __forceinline LPSTR FormatFileLine(LPWSTR FileName, DWORD LineNumber, ULONG FileLength, ULONG BufLength, LPSTR p, BOOL Console)
{
    ULONG UTF8StringActualByteCount;

    // Convert file name from Unicode to UTF-8
    RtlUnicodeToUTF8N(p, BufLength,
        &UTF8StringActualByteCount, FileName, FileLength);
    p += UTF8StringActualByteCount;

    // Optionally apply color formatting
    if (Console)
    {
        memcpy(p, CONSOLE_DEFAULT_FORMAT,
            strlen(CONSOLE_DEFAULT_FORMAT));
        p += strlen(CONSOLE_DEFAULT_FORMAT);
    }

    *p = ':'; // Add a colon separator
    p = dtoa(LineNumber, p + 1);
    *p = '\n'; // Terminate with a newline

    return p + 1;
}

static __forceinline LPSTR FormatSourceCode(PPEB_LDR_DATA Ldr, LPWSTR FileName, DWORD LineNumber, size_t FileLength, ULONG BufLength, LPSTR p, BOOL verbose)
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
                    p = dtoa(line, p);
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
    } else if (verbose >= 1)
    {  // Handle file not found error in verbose mode
        PMESSAGE_RESOURCE_ENTRY Entry;
        ULONG UTF8StringActualByteCount;

        FindNativeMessage(Ldr, ERROR_FILE_NOT_FOUND, LANG_USER_DEFAULT, &Entry);
        // Convert error message to UTF-8
        RtlUnicodeToUTF8N(p, BufLength, &UTF8StringActualByteCount,
            (PCWCH) Entry->Text, Entry->Length - 8);
        p += UTF8StringActualByteCount;
    }

    return p;
}