// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>

class Example {
public:
    // Public method to trigger an exception
    void CauseException(void) {
        RootException();  // Call the private method to cause a breakpoint
    }

private:
    // Static inline method that triggers a breakpoint in the debugger
    static inline void RootException(void) {
        // Cause a breakpoint (used for debugging purposes)
        DebugBreak();
    }
};

int main(void) {
    // Create an instance of the Example class
    Example example;

    // Call the method that will trigger a breakpoint
    example.CauseException();

    // Output a message to the debugger (this will be visible in the debugger's output window)
    OutputDebugStringW(L"Ignored breakpoint\n");

    return 0;  // Return 0 to indicate successful execution
}