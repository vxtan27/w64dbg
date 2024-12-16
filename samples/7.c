/* Copyright (c) 2024, vxtan27. Licensed under the BSD-3-Clause License. */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int main(void)
{
    typedef void(*CallbackFunc)(void);
    CallbackFunc invalidCallback = NULL;
    OutputDebugStringW(L"Accessing DEP\n");
    invalidCallback(); // Dereference a null function pointer
}