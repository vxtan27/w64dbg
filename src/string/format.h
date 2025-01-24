/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

#pragma once

static
__forceinline
char* FormatFileLine(PWSTR FileName, DWORD LineNumber, ULONG len, char* p, char Color)
{
    ULONG UTF8StringActualByteCount;

    // Convert file name from Unicode to UTF-8
    RtlUnicodeToUTF8N(p, BUFLEN,
        &UTF8StringActualByteCount, FileName, len << 1);
    p += UTF8StringActualByteCount;

    // Optionally apply color formatting
    if (Color)
    {
        memcpy(p, CONSOLE_DEFAULT_FORMAT,
            sizeof(CONSOLE_DEFAULT_FORMAT));
        p += sizeof(CONSOLE_DEFAULT_FORMAT);
    }

    *p = ':'; // Add a colon separator
    p = _ultoa10(LineNumber, p + 1);
    *p = '\n'; // Terminate with a newline

    return p + 1;
}

static
__forceinline
char* FormatSourceCode(PWSTR FileName, DWORD LineNumber, size_t _len, char* _buffer, char* p, char verbose)
{
    HANDLE hFile;
    UNICODE_STRING String;
    IO_STATUS_BLOCK IoStatusBlock;

    // Prepend Object Manager namespace to the file name
    memcpy(FileName - 4, OBJECT_MANAGER_NAMESPACE,
        OBJECT_MANAGER_NAMESPACE_LEN);
    String.Length = (_len << 1) + OBJECT_MANAGER_NAMESPACE_LEN;
    String.Buffer = FileName - 4;

    // Open the file with necessary permissions
    NtOpenFile(&hFile, FILE_READ_DATA | SYNCHRONIZE,
        &(OBJECT_ATTRIBUTES) {sizeof(OBJECT_ATTRIBUTES), NULL, &String, OBJ_CASE_INSENSITIVE, NULL, NULL},
        &IoStatusBlock, 0, FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT);

    if (IoStatusBlock.Information == 1) // FILE_OPENED
    {
        char* ptr;
        char buffer[PAGESIZE];
        DWORD line = 1;

        while (TRUE)
        { // Read file content in chunks
            if (NtReadFile(hFile, NULL, NULL, NULL, &IoStatusBlock,
                buffer, PAGESIZE, NULL, NULL)) break;

            ptr = buffer;

            while ((ptr = (char*) memchr(ptr, '\n', buffer + IoStatusBlock.Information - ptr) + 1) > (char*) 1)
            { // Locate and process line breaks
                if (++line == LineNumber)
                {
                    char* _ptr;
                    size_t temp;

                    // Format line number and spacing
                    p = _ultoa10(line, p);
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
                            PAGESIZE, NULL, NULL)) break;
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
    } else if (verbose >= 3)
    {  // Handle file not found error in verbose mode
        DWORD len;
        wchar_t Tmp[WBUFLEN];
        MESSAGE_RESOURCE_ENTRY *Entry;
        ULONG UTF8StringActualByteCount;

        FindSystemMessage(ERROR_FILE_NOT_FOUND, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &Entry);
        // Convert error message to UTF-8
        RtlUnicodeToUTF8N(p, _buffer + BUFLEN - p,
            &UTF8StringActualByteCount, Entry->Text, Entry->Length - 8);
        p += UTF8StringActualByteCount;
    }

    return p;
}