/* Copyright (c) 2024, vxtan27. Licensed under the BSD-3-Clause License. */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

int main(void)
{
    CONTEXT ctx;

    // Enable the Trap Flag for single-step
    ctx.ContextFlags = CONTEXT_CONTROL;
    ctx.EFlags |= 0x100;

    printf("Enabling single-step trap...\n");

    // Cause trace trap
    SetThreadContext(GetCurrentThread(), &ctx);
}