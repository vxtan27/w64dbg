// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

#include <ntdll.h>

// Console Driver IOCTL Definitions
// Reference: https://github.com/microsoft/terminal/blob/main/dep/Console/condrv.h

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

// Console API Message Definitions (Layer 1)
// Reference: https://github.com/microsoft/terminal/blob/main/dep/Console/conmsgl1.h

#define CONSOLE_FIRST_API_NUMBER(Layer) (Layer << 24) \

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

// Set console device mode
NTSTATUS SetConsoleDeviceMode(
    _In_ HANDLE hConsole,
    _In_ UINT dwMode
) {
    CD_USER_DEFINED_IO IoBuffer;
    IoBuffer.InputCount = 1;
    IoBuffer.OutputCount = 0;

    CONSOLE_MSG_MODE_L1 Msg;
    Msg.Header.ApiNumber = ConsolepSetMode;
    Msg.Header.ApiDescriptorSize = sizeof(CONSOLE_MODE_MSG);
    Msg.Msg.Mode = dwMode;

    IoBuffer.Buffers[0].Size = sizeof(Msg);
    IoBuffer.Buffers[0].Buffer = &Msg;

    IO_STATUS_BLOCK IoStatus;
    return NtDeviceIoControlFile(hConsole, NULL, NULL, NULL, &IoStatus,
        IOCTL_CONDRV_ISSUE_USER_IO, &IoBuffer, sizeof(IoBuffer), NULL, 0);
}

// Read console device input
NTSTATUS ReadConsoleDeviceInput(
    _In_ HANDLE hConsole,
    _Out_writes_(nLength) PINPUT_RECORD pBuffer,
    _In_ ULONG nLength,
    _Out_ PULONG64 nEventsRead,
    _In_ BOOL fUnicode
) {
    BYTE Buffer[sizeof(CD_USER_DEFINED_IO) + sizeof(CD_IO_BUFFER) * 2];
    PCD_USER_DEFINED_IO pIoBuffer = (PCD_USER_DEFINED_IO) Buffer;

    pIoBuffer->InputCount = 1;
    pIoBuffer->OutputCount = 2;

    CONSOLE_MSG_GETCONSOLEINPUT_L1 Msg;
    Msg.Header.ApiNumber = ConsolepGetConsoleInput;
    Msg.Header.ApiDescriptorSize = sizeof(CONSOLE_GETCONSOLEINPUT_MSG);
    Msg.Msg.Flags = 0;
    Msg.Msg.Unicode = fUnicode;

    pIoBuffer->Buffers[0].Size = sizeof(Msg);
    pIoBuffer->Buffers[0].Buffer = &Msg;

    pIoBuffer->Buffers[1].Size = sizeof(nEventsRead);
    pIoBuffer->Buffers[1].Buffer = nEventsRead;

    pIoBuffer->Buffers[2].Size = nLength * sizeof(INPUT_RECORD);
    pIoBuffer->Buffers[2].Buffer = pBuffer;

    IO_STATUS_BLOCK IoStatus;
    return NtDeviceIoControlFile(hConsole, NULL, NULL, NULL,
        &IoStatus, IOCTL_CONDRV_ISSUE_USER_IO, pIoBuffer, sizeof(Buffer), NULL, 0);
}

// Write console device output
NTSTATUS WriteConsoleDevice(
    _In_ HANDLE hConsole,
    _In_reads_bytes_(uLength) PCVOID pBuffer,
    _In_ ULONG uLength,
    _In_ BOOL fUnicode
) {
    BYTE Buffer[sizeof(CD_USER_DEFINED_IO) + sizeof(CD_IO_BUFFER)];
    PCD_USER_DEFINED_IO pIoBuffer = (PCD_USER_DEFINED_IO) Buffer;

    pIoBuffer->InputCount = 2;
    pIoBuffer->OutputCount = 0;

    CONSOLE_MSG_WRITECONSOLE_L1 Msg;
    Msg.Header.ApiNumber = ConsolepWriteConsole;
    Msg.Header.ApiDescriptorSize = sizeof(CONSOLE_WRITECONSOLE_MSG);
    Msg.Msg.Unicode = fUnicode;

    pIoBuffer->Buffers[0].Size = sizeof(Msg);
    pIoBuffer->Buffers[0].Buffer = &Msg;

    pIoBuffer->Buffers[1].Size = uLength;
    pIoBuffer->Buffers[1].Buffer = (PVOID) pBuffer;

    IO_STATUS_BLOCK IoStatus;
    return NtDeviceIoControlFile(hConsole, NULL, NULL, NULL,
        &IoStatus, IOCTL_CONDRV_ISSUE_USER_IO, pIoBuffer, sizeof(Buffer), NULL, 0);
}

// Console API Message Definitions (Layer 2)
// Reference: https://github.com/microsoft/terminal/blob/main/dep/Console/conmsgl2.h

typedef struct _CONSOLE_SETCP_MSG {
    IN ULONG CodePage;
    IN BOOLEAN Output;
} CONSOLE_SETCP_MSG, *PCONSOLE_SETCP_MSG;

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

typedef struct _CONSOLE_MSG_SETCP_L2 {
    CONSOLE_MSG_HEADER Header;
    CONSOLE_SETCP_MSG Msg;
} CONSOLE_MSG_SETCP_L2, *PCONSOLE_MSG_SETCP_L2;

// Set console device output code page
NTSTATUS SetConsoleDeviceOutputCP(
    _In_ HANDLE hConsole,
    _In_ UINT wCodePageID
) {
    CD_USER_DEFINED_IO IoBuffer;

    IoBuffer.InputCount = 1;
    IoBuffer.OutputCount = 0;

    CONSOLE_MSG_SETCP_L2 Msg;
    Msg.Header.ApiNumber = ConsolepSetCP;
    Msg.Header.ApiDescriptorSize = sizeof(CONSOLE_SETCP_MSG);
    Msg.Msg.CodePage = wCodePageID;
    Msg.Msg.Output = TRUE;

    IoBuffer.Buffers[0].Size = sizeof(Msg);
    IoBuffer.Buffers[0].Buffer = &Msg;

    IO_STATUS_BLOCK IoStatus;
    return NtDeviceIoControlFile(hConsole, NULL, NULL, NULL, &IoStatus,
        IOCTL_CONDRV_ISSUE_USER_IO, &IoBuffer, sizeof(IoBuffer), NULL, 0);
}