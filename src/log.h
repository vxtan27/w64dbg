// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

#include "fmt.h"

#define DEBUG_EVENT_NAME_MAX_LEN 18
#define TRACE_DEBUG_EVENT_BUFFER_SIZE (DEBUG_EVENT_NAME_MAX_LEN + 22)

//
//  Write a formatted debug event message to the specified handle
//  Utilize a preallocated buffer to minimize runtime allocations
//

NTSTATUS TraceDebugEvent(
    PDBGUI_WAIT_STATE_CHANGE pStateChange,
    PCSTR szDebugEventName,
    SIZE_T DebugEventNameLength,
    HANDLE hStdout,
    BOOL fConsole
) {
    char Buffer[TRACE_DEBUG_EVENT_BUFFER_SIZE];

    return WriteHandle(hStdout, Buffer, FormatDebugEvent(pStateChange,
        szDebugEventName, DebugEventNameLength, Buffer), FALSE, fConsole);
}

//
//  Log module-related debug events
//  Use a MAX_PATH-sized buffer to accommodate typical module names
//

NTSTATUS TraceDebugModule(
    HANDLE hModule,
    PCSTR szDebugEventName,
    SIZE_T DebugEventNameLength,
    HANDLE hStdout,
    BOOL fConsole
) {
    char Buffer[MAX_PATH];

    return WriteHandle(hStdout, Buffer, FormatDebugModule(hModule,
        szDebugEventName, DebugEventNameLength, Buffer), FALSE, fConsole);
}

#pragma push_macro("ExceptionRecord")
#pragma push_macro("ExceptionCode")
#pragma push_macro("RemainingSize")
#pragma push_macro("BaseAddress")

#define ExceptionRecord pStateChange->StateInfo.Exception.ExceptionRecord
#define ExceptionCode ExceptionRecord.ExceptionCode
#define RemainingSize ExceptionRecord.ExceptionInformation[0]
#define BaseAddress ExceptionRecord.ExceptionInformation[1]

// Process OutputDebugString events
VOID ProcessOutputDebugStringEvent(
    PDBGUI_WAIT_STATE_CHANGE pStateChange,
    HANDLE hProcess,
    HANDLE hStdout,
    BOOL fConsole
) {
    // Exclude trailing null character
    --RemainingSize;

    BYTE Buffer[PAGE_SIZE];
    SIZE_T BytesToRead;

    if (ExceptionCode == DBG_PRINTEXCEPTION_WIDE_C) {
        RemainingSize <<= 1;

        BYTE Temp[(PAGE_SIZE >> 1) * 3];
        while (RemainingSize) {
            BytesToRead = PAGE_SIZE - (BaseAddress & PAGE_MASK);

            if (RemainingSize < BytesToRead)
                BytesToRead = RemainingSize;

            if (NtReadVirtualMemory(hProcess, (PVOID) BaseAddress,
                Temp, BytesToRead, NULL) != STATUS_SUCCESS) return;
            WriteHandle(hStdout, Buffer, ConvertUnicodeToUTF8(Temp,
                BytesToRead, Buffer, sizeof(Buffer)), FALSE, fConsole);

            BaseAddress += BytesToRead;
            RemainingSize -= BytesToRead;
        }

        return;
    }

    while (RemainingSize) {
        BytesToRead = PAGE_SIZE - (BaseAddress & PAGE_MASK);

        if (RemainingSize < BytesToRead)
            BytesToRead = RemainingSize;

        if (NtReadVirtualMemory(hProcess, (PVOID) BaseAddress,
            Buffer, BytesToRead, NULL) != STATUS_SUCCESS) return;
        WriteHandle(hStdout, Buffer, BytesToRead, FALSE, fConsole);

        BaseAddress += BytesToRead;
        RemainingSize -= BytesToRead;
    }
}

#undef ExceptionRecord
#undef ExceptionCode
#undef RemainingSize
#undef BaseAddress

#pragma pop_macro("ExceptionRecord")
#pragma pop_macro("ExceptionCode")
#pragma pop_macro("RemainingSize")
#pragma pop_macro("BaseAddress")

#define DEBUG_EVENT_RIP_SLE_MAX_LEN 102
#define DEBUG_EVENT_RIP_BUFFER_SIZE (DEBUG_EVENT_RIP_SLE_MAX_LEN + 900)

// Process RIP (Debugger Error) events and writes diagnostic information
NTSTATUS ProcessRIPEvent(
    PDBGUI_WAIT_STATE_CHANGE pStateChange,
    HANDLE hStdout,
    BOOL fConsole
) {
    char Buffer[DEBUG_EVENT_RIP_BUFFER_SIZE];

    return WriteHandle(hStdout, Buffer,
        FormatRIPEvent(&pStateChange->StateInfo.Exception.ExceptionRecord,
            Buffer, sizeof(Buffer)), FALSE, fConsole);
}