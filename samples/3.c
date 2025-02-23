/*
    Copyright (c) 2024-2025 Xuan Tan. All rights reserved.
    Licensed under the BSD-3-Clause.
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

int main(void)
{
    CONTEXT ctx;

    // Initialize the context structure
    ctx.ContextFlags = CONTEXT_CONTROL;  // Specify that we are modifying control registers

    // Enable the Trap Flag for single-step (0x100 is the mask for the Trap Flag in EFlags)
    ctx.EFlags |= 0x100;

    printf("Enabling single-step trap...\n");

    // Set the current thread's context to enable single-stepping (trace trap)
    if (SetThreadContext(GetCurrentThread(), &ctx) == 0) {
        // Check if the SetThreadContext call was successful
        printf("Failed to set thread context.\n");
        return 1;
    }

    return 0;  // Successful execution
}