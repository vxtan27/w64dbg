/*
    Copyright (c) 2024-2025 Xuan Tan. All rights reserved.
    Licensed under the BSD-3-Clause.
*/

#pragma once

#define DEBUG_EVENT_RIP_SLE_MAX_LEN 102
#define DEBUG_EVENT_RIP_BUFFER_SIZE (DEBUG_EVENT_RIP_SLE_MAX_LEN + 900)

static
FORCEINLINE
NTSTATUS
HandleRIPEvent(
    LPDEBUG_EVENT lpDebugEvent,
    PPEB_LDR_DATA Ldr,
    HANDLE        hStdout)
{
    IO_STATUS_BLOCK IoStatusBlock;
    char Buffer[DEBUG_EVENT_RIP_BUFFER_SIZE];

    return NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock, Buffer,
        FormatRIPEvent(lpDebugEvent, Ldr, Buffer, DEBUG_EVENT_RIP_BUFFER_SIZE), NULL, NULL);
}