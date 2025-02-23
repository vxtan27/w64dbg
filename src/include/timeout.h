/*
    Copyright (c) 2024-2025 Xuan Tan. All rights reserved.
    Licensed under the BSD-3-Clause.
*/

static __forceinline long process_timeout(wchar_t *str, wchar_t **p, size_t len)
{
    BOOL is_signed = FALSE; // Tracks if the value is negative

    // Handle optional sign at the start
    while (*str == '-' || *str == '+')
    {
        is_signed = *str == '-';
        ++str;
    }

    long value = 0;
    unsigned char c;

    do
    { // Parse numeric characters until a space is encountered
        c = *str - '0'; // Normalize to numeric range
        if (c > 9)
        { // Non-numeric character validation
            *p = wmemchr(str, ' ', len) + 1;
            return INVALID_TIMEOUT;
        }
        value = value * 10 + c;
    } while (*++str != ' ');

    *p = str + 1; // Update pointer to next position after the space

    if (is_signed) value = -value;

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

typedef struct
{
    HANDLE hStdout;
    long timeout;
} THREAD_PARAMETER, *PTHREAD_PARAMETER;

static VOID CALLBACK TimerRoutine(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
    long temp;
    ULONG Length;
    char buffer[8];
    DWORD Count, Recursive;
    PTHREAD_PARAMETER Parameter = (PTHREAD_PARAMETER) lpParameter;

    buffer[0] = '\b';
    temp = --Parameter->timeout;
    Count = 1;

    while (temp % 10 + '0' == '9')
    {
        buffer[Count++] = '\b';
        temp /= 10;
    }

    Recursive = Count;

    if (!temp)
    {
        buffer[Count++] = '0';
        --Recursive;
    }

    temp = Parameter->timeout;
    Length = Count + Recursive;

    while (Recursive)
    {
        buffer[Count + --Recursive] = temp % 10 + '0';
        temp /= 10;
    }

    IO_STATUS_BLOCK IoStatusBlock;
    NtWriteFile(Parameter->hStdout, NULL, NULL, NULL,
        &IoStatusBlock, buffer, Length, NULL, NULL);
}

#define InfiniteMessage "\nPress any key to continue ..."
#define _FiniteMessage "\nWaiting for "
#define FiniteMessage_ " seconds, press a key to continue ...\x1b[37D"

static __forceinline VOID WaitForInputOrTimeout(
    HANDLE     hStdin,
    HANDLE     hStdout,
    LONG       timeout,
    BOOL       Console
)
{
    IO_STATUS_BLOCK IoStatusBlock;

    if (timeout == -1)
    {
        // Write the InfiniteMessage
        NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock,
            (PVOID) InfiniteMessage, strlen(InfiniteMessage), NULL, NULL);

        if (Console)
        {
            DWORD dwRead;
            INPUT_RECORD InputRecord;

            do
            { // Infinite loop waiting for valid input
                ReadConsoleInputW(hStdin, &InputRecord, 1, &dwRead);
            } while (IsInputInvalidate(InputRecord));
        } else NtWaitForSingleObject(hStdin, FALSE, NULL);
    } else
    {
        char *p;
        char buffer[64];

        memcpy(buffer, _FiniteMessage, strlen(_FiniteMessage));
        p = jeaiii::to_ascii_chars(buffer + strlen(_FiniteMessage), (unsigned long) timeout, VALID_TIMEOUT);
        memcpy(p, FiniteMessage_, strlen(FiniteMessage_));

        if (Console)
        {
            NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock,
                buffer, p - buffer + strlen(FiniteMessage_), NULL, NULL);

            DWORD dwRead;
            HANDLE hTimer;
            INPUT_RECORD InputRecord;
            LARGE_INTEGER DelayInterval;

            THREAD_PARAMETER Parameter = {hStdout, timeout};
            CreateTimerQueueTimer(&hTimer, NULL, TimerRoutine, &Parameter, 1000, 1000, WT_EXECUTEDEFAULT);
            NtQuerySystemTime(&DelayInterval);
            // Positive value for Absolute timeout
            DelayInterval.QuadPart += SecToMiliSec(timeout);

            do
            { // Wait for either timeout or input
                if (NtWaitForSingleObject(hStdin, FALSE, &DelayInterval) == STATUS_TIMEOUT) break;
                ReadConsoleInputW(hStdin, &InputRecord, 1, &dwRead);
            } while (IsInputInvalidate(InputRecord));
            DeleteTimerQueueTimer(NULL, hTimer, NULL);

        } else
        {
            LARGE_INTEGER DelayInterval = {.QuadPart=-SecToMiliSec(timeout)};
            NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock,
                buffer, p - buffer + strlen(FiniteMessage_) - 5, NULL, NULL);
            NtWaitForSingleObject(hStdin, FALSE, &DelayInterval); // Relative time
        }
    }

    // Simulate normal behavior
    NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock,
        (PVOID) InfiniteMessage, 1, NULL, NULL);
}