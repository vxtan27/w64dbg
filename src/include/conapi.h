// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

#include "ntdll.h"

// https://github.com/microsoft/terminal/blob/main/dep/Console/condrv.h

typedef struct _CD_IO_BUFFER {
    ULONG Size;
    PVOID Buffer;
} CD_IO_BUFFER, *PCD_IO_BUFFER;

typedef struct _CD_USER_DEFINED_IO {
    HANDLE Client;
    ULONG InputCount;
    ULONG OutputCount;
    CD_IO_BUFFER Buffers[ANYSIZE_ARRAY];
} CD_USER_DEFINED_IO, *PCD_USER_DEFINED_IO;

#define IOCTL_CONDRV_ISSUE_USER_IO \
    CTL_CODE(FILE_DEVICE_CONSOLE, 5, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

// https://github.com/microsoft/terminal/blob/main/dep/Console/conmsgl1.h

#define CONSOLE_FIRST_API_NUMBER(Layer) \
    (Layer << 24) \

typedef struct _CONSOLE_MODE_MSG {
    IN OUT ULONG Mode;
} CONSOLE_MODE_MSG, *PCONSOLE_MODE_MSG;

typedef struct _CONSOLE_GETCONSOLEINPUT_MSG {
    OUT ULONG NumRecords;
    IN USHORT Flags;
    IN BOOLEAN Unicode;
} CONSOLE_GETCONSOLEINPUT_MSG, *PCONSOLE_GETCONSOLEINPUT_MSG;

typedef struct _CONSOLE_WRITECONSOLE_MSG {
    OUT ULONG NumBytes;
    IN BOOLEAN Unicode;
} CONSOLE_WRITECONSOLE_MSG, *PCONSOLE_WRITECONSOLE_MSG;

typedef enum _CONSOLE_API_NUMBER_L1 {
    ConsolepGetCP = CONSOLE_FIRST_API_NUMBER(1),
    ConsolepGetMode,
    ConsolepSetMode,
    ConsolepGetNumberOfInputEvents,
    ConsolepGetConsoleInput,
    ConsolepReadConsole,
    ConsolepWriteConsole,
    ConsolepNotifyLastClose,
    ConsolepGetLangId,
    ConsolepMapBitmap,
} CONSOLE_API_NUMBER_L1, *PCONSOLE_API_NUMBER_L1;

typedef struct _CONSOLE_MSG_HEADER {
    ULONG ApiNumber;
    ULONG ApiDescriptorSize;
} CONSOLE_MSG_HEADER, *PCONSOLE_MSG_HEADER;

typedef struct _CONSOLE_MSG_MODE_L1 {
    CONSOLE_MSG_HEADER Header;
    CONSOLE_MODE_MSG Msg;
} CONSOLE_MSG_MODE_L1, *PCONSOLE_MSG_MODE_L1;

typedef struct _CONSOLE_MSG_GETCONSOLEINPUT_L1 {
    CONSOLE_MSG_HEADER Header;
    CONSOLE_GETCONSOLEINPUT_MSG Msg;
} CONSOLE_MSG_GETCONSOLEINPUT_L1, *PCONSOLE_MSG_GETCONSOLEINPUT_L1;

typedef struct _CONSOLE_MSG_WRITECONSOLE_L1 {
    CONSOLE_MSG_HEADER Header;
    CONSOLE_WRITECONSOLE_MSG Msg;
} CONSOLE_MSG_WRITECONSOLE_L1, *PCONSOLE_MSG_WRITECONSOLE_L1;

NTSTATUS ReadConsoleDeviceInput(
    _In_ HANDLE hConsole,
    _Out_writes_(uLength) PINPUT_RECORD pBuffer,
    _In_ ULONG nLength,
    _Out_ PULONG64 nEventsRead,
    _In_ BOOL fUnicode
) {
    // Total size of the data: CD_USER_DEFINED_IO with two additional CD_IO_BUFFER
    BYTE bBuffer[sizeof(CD_USER_DEFINED_IO) + sizeof(CD_IO_BUFFER) * 2];
    PCD_USER_DEFINED_IO pUDIo = (PCD_USER_DEFINED_IO) bBuffer;

    // We not use it
    pUDIo->Client = NULL;
    // Two Input buffers
    pUDIo->InputCount = 1;
    // No Output
    pUDIo->OutputCount = 2;

    CONSOLE_MSG_GETCONSOLEINPUT_L1 Msg = {
        .Header = {ConsolepGetConsoleInput, sizeof(CONSOLE_GETCONSOLEINPUT_MSG)},
        .Msg = {.Unicode=(BOOLEAN)fUnicode}
    };

    // First Input Message Structure
    pUDIo->Buffers[0].Size = sizeof(Msg);
    pUDIo->Buffers[0].Buffer = &Msg;

    // Second Buffer of the text string
    pUDIo->Buffers[1].Size = sizeof(nEventsRead);
    pUDIo->Buffers[1].Buffer = nEventsRead;

    // Third Buffer of the text string
    pUDIo->Buffers[2].Size = nLength * sizeof(INPUT_RECORD);
    pUDIo->Buffers[2].Buffer = pBuffer;

    // Call API
    IO_STATUS_BLOCK IoStatus;
    return NtDeviceIoControlFile(hConsole, NULL, NULL, NULL,
        &IoStatus, IOCTL_CONDRV_ISSUE_USER_IO, pUDIo, sizeof(bBuffer), NULL, 0);
}

// https://www.codeproject.com/Articles/5364085/Tracing-and-Logging-Technologies-on-Windows-Part-3#TOC7_1
NTSTATUS WriteConsoleDevice(
    _In_ HANDLE hConsole,
    _In_reads_bytes_(uLength) PVOID pBuffer,
    _In_ ULONG uLength,
    _In_ BOOL fUnicode
) {
    // Total size of the data: CD_USER_DEFINED_IO with CD_IO_BUFFER
    BYTE bBuffer[sizeof(CD_USER_DEFINED_IO) + sizeof(CD_IO_BUFFER)];
    PCD_USER_DEFINED_IO pUDIo = (PCD_USER_DEFINED_IO) bBuffer;

    // We not use it
    pUDIo->Client = NULL;
    // Two Input buffers
    pUDIo->InputCount = 2;
    // No Output
    pUDIo->OutputCount = 0;

    CONSOLE_MSG_WRITECONSOLE_L1 Msg = {
        .Header = {ConsolepWriteConsole, sizeof(CONSOLE_WRITECONSOLE_MSG)},
        .Msg = {.Unicode=(BOOLEAN)fUnicode}
    };

    // First Input Message Structure
    pUDIo->Buffers[0].Size = sizeof(Msg);
    pUDIo->Buffers[0].Buffer = &Msg;

    // Second Buffer of the text string
    pUDIo->Buffers[1].Size = uLength;
    pUDIo->Buffers[1].Buffer = pBuffer;

    // Call API
    IO_STATUS_BLOCK IoStatus;
    return NtDeviceIoControlFile(hConsole, NULL, NULL, NULL,
        &IoStatus, IOCTL_CONDRV_ISSUE_USER_IO, pUDIo, sizeof(bBuffer), NULL, 0);
}

NTSTATUS SetConsoleDeviceMode(
    _In_ HANDLE hConsole,
    _In_ UINT dwMode
) {
    CD_USER_DEFINED_IO UDIo;

    // We not use it
    UDIo.Client = NULL;
    // Two Input buffers
    UDIo.InputCount = 1;
    // No Output
    UDIo.OutputCount = 0;

    CONSOLE_MSG_MODE_L1 Msg = {
        .Header = {ConsolepSetMode, sizeof(CONSOLE_MODE_MSG)},
        .Msg = {dwMode}
    };

    // First Input Message Structure
    UDIo.Buffers[0].Size = sizeof(Msg);
    UDIo.Buffers[0].Buffer = &Msg;

    // Call API
    IO_STATUS_BLOCK IoStatus;
    return NtDeviceIoControlFile(hConsole, NULL, NULL, NULL, &IoStatus,
        IOCTL_CONDRV_ISSUE_USER_IO, &UDIo, sizeof(UDIo), NULL, 0);
}

// https://github.com/microsoft/terminal/blob/main/dep/Console/conmsgl2.h

typedef struct _CONSOLE_SETCP_MSG {
    IN ULONG CodePage;
    IN BOOLEAN Output;
} CONSOLE_SETCP_MSG, *PCONSOLE_SETCP_MSG;

typedef struct _CONSOLE_MSG_SETCP_L2 {
    CONSOLE_MSG_HEADER Header;
    CONSOLE_SETCP_MSG Msg;
} CONSOLE_MSG_SETCP_L2, *PCONSOLE_MSG_SETCP_L2;

typedef enum _CONSOLE_API_NUMBER_L2 {
    ConsolepFillConsoleOutput = CONSOLE_FIRST_API_NUMBER(2),
    ConsolepGenerateCtrlEvent,
    ConsolepSetActiveScreenBuffer,
    ConsolepFlushInputBuffer,
    ConsolepSetCP,
    ConsolepGetCursorInfo,
    ConsolepSetCursorInfo,
    ConsolepGetScreenBufferInfo,
    ConsolepSetScreenBufferInfo,
    ConsolepSetScreenBufferSize,
    ConsolepSetCursorPosition,
    ConsolepGetLargestWindowSize,
    ConsolepScrollScreenBuffer,
    ConsolepSetTextAttribute,
    ConsolepSetWindowInfo,
    ConsolepReadConsoleOutputString,
    ConsolepWriteConsoleInput,
    ConsolepWriteConsoleOutput,
    ConsolepWriteConsoleOutputString,
    ConsolepReadConsoleOutput,
    ConsolepGetTitle,
    ConsolepSetTitle,
} CONSOLE_API_NUMBER_L2, *PCONSOLE_API_NUMBER_L2;

NTSTATUS SetConsoleDeviceOutputCP(
    _In_ HANDLE hConsole,
    _In_ UINT wCodePageID
) {
    CD_USER_DEFINED_IO UDIo;

    // We not use it
    UDIo.Client = NULL;
    // Two Input buffers
    UDIo.InputCount = 1;
    // No Output
    UDIo.OutputCount = 0;

    CONSOLE_MSG_SETCP_L2 Msg = {
        .Header = {ConsolepSetCP, sizeof(CONSOLE_SETCP_MSG)},
        .Msg = {wCodePageID, TRUE}
    };

    // First Input Message Structure
    UDIo.Buffers[0].Size = sizeof(Msg);
    UDIo.Buffers[0].Buffer = &Msg;

    // Call API
    IO_STATUS_BLOCK IoStatus;
    return NtDeviceIoControlFile(hConsole, NULL, NULL, NULL, &IoStatus,
        IOCTL_CONDRV_ISSUE_USER_IO, &UDIo, sizeof(UDIo), NULL, 0);
}