// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int main(void) {
    // Define a function pointer type for callback functions
    typedef void(*CallbackFunc)(void);

    // Initialize the function pointer to NULL (invalid callback)
    CallbackFunc invalidCallback = NULL;

    // Output a message to the debugger (this will be visible in the debugger's output window)
    OutputDebugStringW(L"Accessing DEP (Data Execution Prevention)\n");

    // Dereference the null function pointer, causing a crash (undefined behavior)
    invalidCallback();  // This will lead to a crash as the function pointer is NULL

    return 0;  // Return 0 (although it won't be reached due to the crash)
}