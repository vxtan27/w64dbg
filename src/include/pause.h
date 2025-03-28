// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

#define IsInputInvalidate(InputRecord) ( \
    InputRecord.EventType != KEY_EVENT || \
    !InputRecord.Event.KeyEvent.bKeyDown || \
    ( \
        InputRecord.Event.KeyEvent.wVirtualKeyCode < 0x30 && \
        InputRecord.Event.KeyEvent.wVirtualKeyCode != VK_RETURN && \
        InputRecord.Event.KeyEvent.wVirtualKeyCode != VK_ESCAPE && \
        InputRecord.Event.KeyEvent.wVirtualKeyCode != VK_PAUSE) || \
    ( \
        InputRecord.Event.KeyEvent.wVirtualKeyCode > 0x5A && \
        InputRecord.Event.KeyEvent.wVirtualKeyCode < VK_SLEEP) || \
    ( \
        InputRecord.Event.KeyEvent.wVirtualKeyCode > VK_DIVIDE && \
        InputRecord.Event.KeyEvent.wVirtualKeyCode < VK_OEM_1) || \
    InputRecord.Event.KeyEvent.wVirtualKeyCode > VK_OEM_102 \
)

#define InfiniteMessage "\nPress any key to continue ..."

VOID WaitOrTimeout(
    HANDLE     hStdout,
    BOOL       bConsole
) {
    // Write the InfiniteMessage
    WriteHandle(hStdout, (PVOID) InfiniteMessage, strlen(InfiniteMessage), FALSE, bConsole);

    HANDLE hStdin = RtlStandardInput();

    if (bConsole) {
        ULONG64 dwRead;
        INPUT_RECORD InputRecord;

        do { // Infinite loop waiting for valid input
            ReadConsoleDeviceInput(hStdin, &InputRecord, 1, &dwRead, TRUE);
        } while (IsInputInvalidate(InputRecord));
    } else NtWaitForSingleObject(hStdin, FALSE, NULL);

    // Simulate normal behavior
    WriteHandle(hStdout, (PVOID) InfiniteMessage, 1, FALSE, bConsole);
}