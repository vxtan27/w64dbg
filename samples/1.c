/* Copyright (c) 2024, vxtan27. Licensed under the BSD-3-Clause License. */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

int main(void)
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    DWORD oldProtect;
    LPVOID guardPage;
    guardPage = VirtualAlloc(NULL, si.dwPageSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    VirtualProtect(guardPage, si.dwPageSize, PAGE_GUARD | PAGE_READWRITE, &oldProtect);

    printf("Accessing memory page...\n");
    // Cause guard page exception
    *(volatile char *) guardPage = 1;
}
