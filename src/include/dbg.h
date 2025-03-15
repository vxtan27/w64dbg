//
//  SPDX-License-Identifier: BSD-3-Clause
//  Copyright (c) 2024-2025 Xuan Tan. All rights reserved.
//

#pragma once

// Waits for a debugging event to occur in a process being debugged

static
FORCEINLINE
NTSTATUS
DbgWaitForDebugEvent(PDBGUI_WAIT_STATE_CHANGE pStateChange, PLARGE_INTEGER pTimeout)
{
    // NtCurrentTeb()->DbgSsReserved[0]
    return NtWaitForDebugEvent(NtCurrentTeb()->Reserved5[4], FALSE, pTimeout, pStateChange);
}

// Enables a debugger to continue a thread that previously reported a debugging event

static
FORCEINLINE
BOOL
DbgContinueDebugEvent(PDBGUI_WAIT_STATE_CHANGE pStateChange, DWORD dwContinueStatus)
{
    return ContinueDebugEvent(HandleToUlong(pStateChange->AppClientId.UniqueProcess),
            HandleToUlong(pStateChange->AppClientId.UniqueThread), dwContinueStatus);
}

// Handles OutputDebugString (ODS) events and writes the debug string to standard output

static
FORCEINLINE
VOID
DbgProcessODS(
    PDBGUI_WAIT_STATE_CHANGE pStateChange,
    HANDLE                   hProcess,
    HANDLE                   hStdout)
{
    char Buffer[BUFLEN];
    SIZE_T NumberOfBytesToRead;
    IO_STATUS_BLOCK IoStatusBlock;

    --pStateChange->StateInfo.Exception.ExceptionRecord.ExceptionInformation[0];

    if (pStateChange->StateInfo.Exception.ExceptionRecord.ExceptionCode == DBG_PRINTEXCEPTION_WIDE_C)
    {
        ULONG ActualByteCount;
        wchar_t Temp[BUFLEN >> 2];

        pStateChange->StateInfo.Exception.ExceptionRecord.ExceptionInformation[0] <<= 1;

        while (pStateChange->StateInfo.Exception.ExceptionRecord.ExceptionInformation[0] > 0)
        {
            NumberOfBytesToRead = pStateChange->StateInfo.Exception.ExceptionRecord.ExceptionInformation[0] < sizeof(Temp)
                                ? pStateChange->StateInfo.Exception.ExceptionRecord.ExceptionInformation[0] : sizeof(Temp);

            NtReadVirtualMemory(hProcess,
                (PVOID) pStateChange->StateInfo.Exception.ExceptionRecord.ExceptionInformation[1], Temp, NumberOfBytesToRead, NULL);
            RtlUnicodeToUTF8N(Buffer, BUFLEN, &ActualByteCount, Temp, NumberOfBytesToRead);
            NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock, Buffer, ActualByteCount, NULL, NULL);

            pStateChange->StateInfo.Exception.ExceptionRecord.ExceptionInformation[1] += NumberOfBytesToRead;
            pStateChange->StateInfo.Exception.ExceptionRecord.ExceptionInformation[0] -= NumberOfBytesToRead;
        }
    } else while (pStateChange->StateInfo.Exception.ExceptionRecord.ExceptionInformation[0] > 0)
    {
        NumberOfBytesToRead = pStateChange->StateInfo.Exception.ExceptionRecord.ExceptionInformation[0] < sizeof(Buffer)
                            ? pStateChange->StateInfo.Exception.ExceptionRecord.ExceptionInformation[0] : sizeof(Buffer);

        NtReadVirtualMemory(hProcess,
            (PVOID) pStateChange->StateInfo.Exception.ExceptionRecord.ExceptionInformation[1], Buffer, NumberOfBytesToRead, NULL);
        NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock, Buffer, NumberOfBytesToRead, NULL, NULL);

        pStateChange->StateInfo.Exception.ExceptionRecord.ExceptionInformation[1] += NumberOfBytesToRead;
        pStateChange->StateInfo.Exception.ExceptionRecord.ExceptionInformation[0] -= NumberOfBytesToRead;
    }
}

#define DEBUG_EVENT_RIP_SLE_MAX_LEN 102
#define DEBUG_EVENT_RIP_BUFFER_SIZE (DEBUG_EVENT_RIP_SLE_MAX_LEN + 900)

// Handles RIP (Debugger Error) events and writes diagnostic information

static
FORCEINLINE
NTSTATUS
DbgProcessRIP(
    PDBGUI_WAIT_STATE_CHANGE pStateChange,
    PPEB_LDR_DATA            Ldr,
    HANDLE                   hStdout)
{
    IO_STATUS_BLOCK IoStatusBlock;
    char Buffer[DEBUG_EVENT_RIP_BUFFER_SIZE];

    return NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock, Buffer,
        DbgFormatRIP(pStateChange, Ldr, Buffer, sizeof(Buffer)), NULL, NULL);
}