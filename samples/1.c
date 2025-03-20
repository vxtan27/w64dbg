// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

int main(void) {
    SYSTEM_INFO si;

    // Retrieve system information (specifically page size)
    GetSystemInfo(&si);

    DWORD oldProtect;
    LPVOID guardPage;

    // Allocate memory with page size of the system
    guardPage = VirtualAlloc(
        NULL,                       // Address of the memory to allocate
        si.dwPageSize,              // Size of the page to allocate
        MEM_COMMIT | MEM_RESERVE,    // Allocation type (commit and reserve)
        PAGE_READWRITE              // Memory protection (read/write)
    );

    if (guardPage == NULL) {
        // Check if memory allocation failed
        printf("Memory allocation failed\n");
        return 1;
    }

    // Set the allocated page as a guard page
    VirtualProtect(
        guardPage,                  // Address of the memory page
        si.dwPageSize,              // Size of the page
        PAGE_GUARD | PAGE_READWRITE, // Protection flags (guard and read/write)
        &oldProtect                 // Store the old protection settings
    );

    printf("Accessing memory page...\n");

    // Attempt to access the guard page (this will cause a guard page exception)
    *(volatile char *)guardPage = 1;

    // The program should not reach here due to the exception being triggered
    printf("This will not be printed unless guard page exception is handled.\n");

    return 0;
}