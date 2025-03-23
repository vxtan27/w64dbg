// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

long process_timeout(wchar_t *str, wchar_t **p, SIZE_T len) {
    long value = 0;
    unsigned char c;

    do { // Parse numeric characters until a space is encountered
        c = *str - '0'; // Normalize to numeric range
        if (c > 9) { // Non-numeric character validation
            *p = wmemchr(str, ' ', len) + 1;
            return INVALID_TIMEOUT;
        }
        value = value * 10 + c;
    } while (*++str != ' ');

    *p = str + 1; // Update pointer to next position after the space

    return value;
}

#define MiliSecToUnits(lSeconds) ((lSeconds) * 10000LL)
#define SecToMiliSec(lSeconds) MiliSecToUnits(lSeconds * 1000)

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

typedef struct {
    HANDLE hStdout;
    DWORD dwTimeout;
    BOOL bConsole;
} THREAD_PARAMETER, *PTHREAD_PARAMETER;

VOID CALLBACK TimerRoutine(PVOID lpParameter, BOOLEAN TimerOrWaitFired) {
    DWORD temp;
    ULONG Length;
    char buffer[8];
    DWORD Count, Recursive;
    PTHREAD_PARAMETER Parameter = (PTHREAD_PARAMETER) lpParameter;

    buffer[0] = '\b';
    temp = --Parameter->dwTimeout;
    Count = 1;

    while (temp % 10 + '0' == '9') {
        buffer[Count++] = '\b';
        temp /= 10;
    }

    Recursive = Count;

    if (!temp) {
        buffer[Count++] = '0';
        --Recursive;
    }

    temp = Parameter->dwTimeout;
    Length = Count + Recursive;

    while (Recursive) {
        buffer[Count + --Recursive] = temp % 10 + '0';
        temp /= 10;
    }

    WriteHandle(Parameter->hStdout, buffer, Length, FALSE, Parameter->bConsole);
}

#define InfiniteMessage "\nPress any key to continue ..."
#define _FiniteMessage "\nWaiting for "
#define FiniteMessage_ " seconds, press a key to continue ...\x1b[37D"

VOID WaitForInputOrTimeout(
    HANDLE     hStdout,
    LONG       dwTimeout,
    BOOL       bConsole
) {
    HANDLE hStdin = RtlStandardInput();

    if (dwTimeout == -1) {
        // Write the InfiniteMessage
        WriteHandle(hStdout, (PVOID) InfiniteMessage, strlen(InfiniteMessage), FALSE, bConsole);

        if (bConsole) {
            ULONG64 dwRead;
            INPUT_RECORD InputRecord;

            do { // Infinite loop waiting for valid input
                ReadConsoleDeviceInput(hStdin, &InputRecord, 1, &dwRead, TRUE);
            } while (IsInputInvalidate(InputRecord));
        } else NtWaitForSingleObject(hStdin, FALSE, NULL);
    } else {
        char *p;
        char buffer[64];

        memcpy(buffer, _FiniteMessage, strlen(_FiniteMessage));
        p = int_to_chars(buffer + strlen(_FiniteMessage), (unsigned long) dwTimeout, VALID_TIMEOUT);
        memcpy(p, FiniteMessage_, strlen(FiniteMessage_));

        if (bConsole) {
            WriteHandle(hStdout, buffer, p - buffer + strlen(FiniteMessage_), FALSE, bConsole);

            ULONG64 dwRead;
            HANDLE hTimer;
            INPUT_RECORD InputRecord;
            LARGE_INTEGER DelayInterval;

            THREAD_PARAMETER Parameter = {hStdout, dwTimeout, bConsole};
            CreateTimerQueueTimer(&hTimer, NULL, TimerRoutine, &Parameter, 1000, 1000, WT_EXECUTEDEFAULT);
            NtQuerySystemTime(&DelayInterval);
            // Positive value for Absolute dwTimeout
            DelayInterval.QuadPart += SecToMiliSec(dwTimeout);

            do { // Wait for either dwTimeout or input
                if (NtWaitForSingleObject(hStdin, FALSE, &DelayInterval) == STATUS_TIMEOUT) break;
                ReadConsoleDeviceInput(hStdin, &InputRecord, 1, &dwRead, TRUE);
            } while (IsInputInvalidate(InputRecord));
            DeleteTimerQueueTimer(NULL, hTimer, NULL);

        } else {
            LARGE_INTEGER DelayInterval = {.QuadPart=-SecToMiliSec(dwTimeout)};
            WriteHandle(hStdout, buffer, p - buffer + strlen(FiniteMessage_) - 5, FALSE, bConsole);
            NtWaitForSingleObject(hStdin, FALSE, &DelayInterval); // Relative time
        }
    }

    // Simulate normal behavior
    WriteHandle(hStdout, (PVOID) InfiniteMessage, 1, FALSE, bConsole);
}