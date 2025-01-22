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

    RtlUnicodeToUTF8N(p, BUFLEN,
        &UTF8StringActualByteCount, FileName, len << 1);
    p += UTF8StringActualByteCount;

    if (Color)
    {
        memcpy(p, CONSOLE_DEFAULT_FORMAT,
            sizeof(CONSOLE_DEFAULT_FORMAT));
        p += sizeof(CONSOLE_DEFAULT_FORMAT);
    }

    *p = ':';
    p = _ultoa10(LineNumber, p + 1);
    *p = '\n';

    return p + 1;
}

// static const wchar_t OBJECT_MANAGER_NAMESPACE[4] = L"\\??\\";
// static const char OBJECT_MANAGER_NAMESPACE[] = "\\\0?\0?\0\\";

#define OBJECT_MANAGER_NAMESPACE GDB_COMMAND_LINE + 14
#define OBJECT_MANAGER_NAMESPACE_LEN 8

static
__forceinline
char* FormatSourceCode(PWSTR FileName, DWORD LineNumber, size_t _len, char* _buffer, char* p, char verbose)
{
    HANDLE hFile;
    UNICODE_STRING String;
    IO_STATUS_BLOCK IoStatusBlock;

    // Windows Object Manager namespace
    memcpy(FileName - 4, OBJECT_MANAGER_NAMESPACE,
        OBJECT_MANAGER_NAMESPACE_LEN);
    String.Length = (_len << 1) + OBJECT_MANAGER_NAMESPACE_LEN;
    String.Buffer = FileName - 4;
    NtCreateFile(&hFile, FILE_READ_DATA | SYNCHRONIZE,
        &(OBJECT_ATTRIBUTES) {sizeof(OBJECT_ATTRIBUTES), NULL, &String, OBJ_CASE_INSENSITIVE, NULL, NULL},
        &IoStatusBlock, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN,
        FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

    if (IoStatusBlock.Information == 1) // FILE_OPENED
    {
        char* ptr;
        char buffer[PAGESIZE];
        DWORD line = 1;

        while (TRUE)
        {
            if (NtReadFile(hFile, NULL, NULL, NULL, &IoStatusBlock,
                buffer, PAGESIZE, NULL, NULL)) break;

            ptr = buffer;

            while ((ptr = (char*) memchr(ptr, '\n', buffer + IoStatusBlock.Information - ptr) + 1) > (char*) 1)
            {
                if (++line == LineNumber)
                {
                    char* _ptr;
                    size_t temp;

                    p = _ultoa10(line, p);
                    memset(p, ' ', 6);
                    p += 6;
                    _ptr = (char*) memchr(ptr, '\n', buffer + IoStatusBlock.Information - ptr);

                    if (_ptr) temp = _ptr - ptr;
                    else
                    {
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

        NtClose(hFile);
    } else if (verbose >= 3)
    {
        wchar_t temp[WBUFLEN];
        ULONG UTF8StringActualByteCount;

        DWORD len = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
            ERROR_FILE_NOT_FOUND, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), temp, WBUFLEN, NULL);
        RtlUnicodeToUTF8N(p, _buffer + BUFLEN - p,
            &UTF8StringActualByteCount, temp, len << 1);
        p += UTF8StringActualByteCount;
    }

    return p;
}