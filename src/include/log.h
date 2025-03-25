// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

#define DEBUG_EVENT_NAME_MAX_LEN 18
#define TRACE_DEBUG_EVENT_BUFFER_SIZE (DEBUG_EVENT_NAME_MAX_LEN + 22)

//
//  Writes a formatted debug event message to the specified handle
//  Utilizes a preallocated buffer to minimize runtime allocations
//

NTSTATUS TraceDebugEvent(
    PDBGUI_WAIT_STATE_CHANGE pStateChange,
    PCSTR szDebugEventName,
    SIZE_T DebugEventNameLength,
    HANDLE hStdout,
    BOOL bConsole
) {
    char Buffer[TRACE_DEBUG_EVENT_BUFFER_SIZE];

    return WriteHandle(hStdout, Buffer, FormatDebugEvent(pStateChange,
        szDebugEventName, DebugEventNameLength, Buffer), FALSE, bConsole);
}

//
//  Logs module-related debug events
//  Uses a MAX_PATH-sized buffer to accommodate typical module names
//

NTSTATUS TraceDebugModule(
    HANDLE hModule,
    PCSTR szDebugEventName,
    SIZE_T DebugEventNameLength,
    HANDLE hStdout,
    BOOL bConsole
) {
    char Buffer[MAX_PATH];

    return WriteHandle(hStdout, Buffer, FormatDebugModule(hModule,
        szDebugEventName, DebugEventNameLength, Buffer), FALSE, bConsole);
}

// Handles OutputDebugString events and writes the debug string to standard output
VOID ProcessDebugStringEvent(
    PDBGUI_WAIT_STATE_CHANGE pStateChange,
    HANDLE hProcess,
    HANDLE hStdout,
    BOOL bConsole
) {
    PEXCEPTION_RECORD pExceptionRecord = &pStateChange->StateInfo.Exception.ExceptionRecord;

    // Exclude trailing null character
    --pExceptionRecord->ExceptionInformation[0];

    if (pExceptionRecord->ExceptionCode == DBG_PRINTEXCEPTION_WIDE_C) {
        pExceptionRecord->ExceptionInformation[0] <<= 1;

        while (pExceptionRecord->ExceptionInformation[0] > 0) {
            char Buffer[BUFLEN];
            ULONG ActualByteCount;
            wchar_t Temp[sizeof(Buffer) / 3];
            SIZE_T BytesToRead = pExceptionRecord->ExceptionInformation[0] < sizeof(Temp)
                                ? pExceptionRecord->ExceptionInformation[0] : sizeof(Temp);

            NtReadVirtualMemory(hProcess,
                (PVOID) pExceptionRecord->ExceptionInformation[1], Temp, BytesToRead, NULL);
            RtlUnicodeToUTF8N(Buffer, sizeof(Buffer), &ActualByteCount, Temp, BytesToRead);
            WriteHandle(hStdout, Buffer, ActualByteCount, FALSE, bConsole);

            pExceptionRecord->ExceptionInformation[1] += BytesToRead;
            pExceptionRecord->ExceptionInformation[0] -= BytesToRead;
        }

        return;
    }

    while (pExceptionRecord->ExceptionInformation[0] > 0) {
        char Buffer[BUFLEN];
        SIZE_T BytesToRead = pExceptionRecord->ExceptionInformation[0] < sizeof(Buffer)
                            ? pExceptionRecord->ExceptionInformation[0] : sizeof(Buffer);

        NtReadVirtualMemory(hProcess,
            (PVOID) pExceptionRecord->ExceptionInformation[1], Buffer, BytesToRead, NULL);
        WriteHandle(hStdout, Buffer, BytesToRead, FALSE, bConsole);

        pExceptionRecord->ExceptionInformation[1] += BytesToRead;
        pExceptionRecord->ExceptionInformation[0] -= BytesToRead;
    }
}

#define DEBUG_EVENT_RIP_SLE_MAX_LEN 102
#define DEBUG_EVENT_RIP_BUFFER_SIZE (DEBUG_EVENT_RIP_SLE_MAX_LEN + 900)

// Handles RIP (Debugger Error) events and writes diagnostic information
NTSTATUS ProcessRIPEvent(
    PDBGUI_WAIT_STATE_CHANGE pStateChange,
    HANDLE hStdout,
    BOOL bConsole
) {
    char Buffer[DEBUG_EVENT_RIP_BUFFER_SIZE];

    return WriteHandle(hStdout, Buffer,
        FormatRIPEvent(pStateChange, Buffer, sizeof(Buffer)), FALSE, bConsole);
}