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
    LPDEBUG_EVENT lpDebugEvent,
    HANDLE        hProcess,
    HANDLE        hStdout)
{
    char Buffer[BUFLEN];
    SIZE_T NumberOfBytesToRead;
    IO_STATUS_BLOCK IoStatusBlock;

    if (lpDebugEvent->u.DebugString.fUnicode)
    {
        ULONG ActualByteCount;
        wchar_t Temp[BUFLEN >> 2];

        lpDebugEvent->u.DebugString.nDebugStringLength -= 2;

        while (lpDebugEvent->u.DebugString.nDebugStringLength > 0)
        {
            NumberOfBytesToRead = lpDebugEvent->u.DebugString.nDebugStringLength < sizeof(Temp)
                                ? lpDebugEvent->u.DebugString.nDebugStringLength : sizeof(Temp);

            NtReadVirtualMemory(hProcess,
                lpDebugEvent->u.DebugString.lpDebugStringData, Temp, NumberOfBytesToRead, NULL);
            RtlUnicodeToUTF8N(Buffer, BUFLEN, &ActualByteCount, Temp, NumberOfBytesToRead);
            NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock, Buffer, ActualByteCount, NULL, NULL);

            lpDebugEvent->u.DebugString.lpDebugStringData += NumberOfBytesToRead;
            lpDebugEvent->u.DebugString.nDebugStringLength -= NumberOfBytesToRead;
        }
    } else
    {
        --lpDebugEvent->u.DebugString.nDebugStringLength;

        while (lpDebugEvent->u.DebugString.nDebugStringLength > 0)
        {
            NumberOfBytesToRead = lpDebugEvent->u.DebugString.nDebugStringLength < sizeof(Buffer)
                                ? lpDebugEvent->u.DebugString.nDebugStringLength : sizeof(Buffer);

            NtReadVirtualMemory(hProcess,
                lpDebugEvent->u.DebugString.lpDebugStringData, Buffer, NumberOfBytesToRead, NULL);
            NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock, Buffer, NumberOfBytesToRead, NULL, NULL);

            lpDebugEvent->u.DebugString.lpDebugStringData += NumberOfBytesToRead;
            lpDebugEvent->u.DebugString.nDebugStringLength -= NumberOfBytesToRead;
        }
    }
}

/* Handles RIP (Debugger Error) events and writes diagnostic information. */

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