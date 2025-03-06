/*
    Copyright (c) 2024-2025 Xuan Tan. All rights reserved.
    Licensed under the BSD-3-Clause.
*/

#pragma once

#define DEBUG_EVENT_NAME_MAX_LEN 18
#define TRACE_DEBUG_EVENT_BUFFER_SIZE (DEBUG_EVENT_NAME_MAX_LEN + 22)

/*
    Writes a formatted debug event message to the specified handle.
    Utilizes a preallocated buffer to minimize runtime allocations.
*/

static
FORCEINLINE
NTSTATUS
TraceDebugEvent(
    LPDEBUG_EVENT lpDebugEvent,
    LPCSTR        szDebugEventName,
    SIZE_T        DebugEventNameLength,
    HANDLE        hStdout)
{
    IO_STATUS_BLOCK IoStatusBlock;
    char Buffer[TRACE_DEBUG_EVENT_BUFFER_SIZE];

    return NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock, Buffer,
        FormatDebugEvent(lpDebugEvent, szDebugEventName, DebugEventNameLength, Buffer), NULL, NULL);
}

/*
    Logs module-related debug events.
    Uses a MAX_PATH-sized buffer to accommodate typical module names.
*/

static
FORCEINLINE
NTSTATUS
TraceModuleEvent(
    LPDEBUG_EVENT lpDebugEvent,
    LPCSTR        szDebugEventName,
    SIZE_T        DebugEventNameLength,
    HANDLE        hStdout)
{
    char Buffer[MAX_PATH];
    IO_STATUS_BLOCK IoStatusBlock;

    return NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock, Buffer,
        FormatModuleEvent(lpDebugEvent, szDebugEventName, DebugEventNameLength, Buffer), NULL, NULL);
}