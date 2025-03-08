/*
    Copyright (c) 2024-2025 Xuan Tan. All rights reserved.
    Licensed under the BSD-3-Clause.
*/

#pragma once

#define DEBUG_EVENT_RIP_SLE_MAX_LEN 102
#define DEBUG_EVENT_RIP_BUFFER_SIZE (DEBUG_EVENT_RIP_SLE_MAX_LEN + 900)

/* Determines whether a given handle is associated with a console device. */

static
FORCEINLINE
NTSTATUS
IsConsoleHandle(HANDLE hHandle, PBOOL Result)
{
    NTSTATUS NtStatus;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_FS_DEVICE_INFORMATION DeviceInfo;

    NtStatus = NtQueryVolumeInformationFile(hHandle, &IoStatusBlock,
        &DeviceInfo, sizeof(DeviceInfo), FileFsDeviceInformation);

    *Result = (DeviceInfo.DeviceType == FILE_DEVICE_CONSOLE);
    return NtStatus;
}

/* Handles OutputDebugString (ODS) events and writes the debug string to standard output. */

static
FORCEINLINE
VOID
HandleODSEvent(
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
    } else
    {
        while (pStateChange->StateInfo.Exception.ExceptionRecord.ExceptionInformation[0] > 0)
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
}

/* Handles RIP (Debugger Error) events and writes diagnostic information. */

static
FORCEINLINE
NTSTATUS
HandleRIPEvent(
    PDBGUI_WAIT_STATE_CHANGE pStateChange,
    PPEB_LDR_DATA            Ldr,
    HANDLE                   hStdout)
{
    IO_STATUS_BLOCK IoStatusBlock;
    char Buffer[DEBUG_EVENT_RIP_BUFFER_SIZE];

    return NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock, Buffer,
        FormatRIPEvent(pStateChange, Ldr, Buffer, DEBUG_EVENT_RIP_BUFFER_SIZE), NULL, NULL);
}

/* Retrieves the size of a loaded module. */

static
FORCEINLINE
DWORD
GetModuleSize(HANDLE hModule)
{
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_STANDARD_INFORMATION FileInfo;

    NtQueryInformationFile(hModule, &IoStatusBlock,
        &FileInfo, sizeof(FileInfo), FileStandardInformation);
    return FileInfo.EndOfFile.LowPart;
}