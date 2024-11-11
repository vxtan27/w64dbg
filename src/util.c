#pragma once

#include <string.h>
#include <stdlib.h>
#include <windows.h>

static inline void _ultoaddr(unsigned long value, char *p, char *buffer)
{
    int temp;
    memset(p, '0', 8);
    _ultoa(value, buffer, 16);
    temp = strlen(buffer);
    memcpy(p + 8 - temp, buffer, temp);
}

static inline void ulltoaddr(unsigned long long value, char *p, char *buffer)
{
    int temp;
    memset(p, '0', 16);
    _ui64toa(value, buffer, 16);
    temp = strlen(buffer);
    memcpy(p + 16 - temp, buffer, temp);
}

static inline DWORD GetModuleSize(HANDLE hFile)
{
    DWORD Size;
    HANDLE hMapping;
    LPVOID lpBaseAddress;
    PIMAGE_DOS_HEADER dosHeader;
    PIMAGE_NT_HEADERS ntHeaders;
    hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    lpBaseAddress = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    dosHeader = (PIMAGE_DOS_HEADER)lpBaseAddress;
    ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)lpBaseAddress + dosHeader->e_lfanew);
    Size = ntHeaders->OptionalHeader.SizeOfImage;
    UnmapViewOfFile(lpBaseAddress);
    CloseHandle(hMapping);
    return Size;
}