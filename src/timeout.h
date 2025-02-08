/*
    Copyright (c) 2024, vxtan27, all rights reserved.
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
            *p = __builtin__wmemchr(str, ' ', len) + 1;
            return INVALID_TIMEOUT;
        }
        value = value * 10 + c;
    } while (*++str != ' ');

    *p = str + 1; // Update pointer to next position after the space

    if (is_signed) value = -value;

    return value;
}

#define MiliSecToUnits(lSeconds) ((lSeconds) * 10000LL)
#define SecToUnits(lSeconds) MiliSecToUnits(lSeconds * 1000)

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

__declspec(noreturn)
static VOID WINAPI WaitForInput(LPVOID lpParameter)
{
    long temp;
    ULONG Length;
    char buffer[8];
    DWORD Count, Recursive;
    IO_STATUS_BLOCK IoStatusBlock;
    PTHREAD_PARAMETER Parameter = lpParameter;

    buffer[0] = '\b';

    while (TRUE)
    {
        NtDelayExecution(FALSE,
            &(LARGE_INTEGER){.QuadPart=-MiliSecToUnits(999)});
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

        NtWriteFile(Parameter->hStdout, NULL, NULL, NULL,
            &IoStatusBlock, buffer, Length, NULL, NULL);
    }

    RtlExitUserThread(0);
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
            InfiniteMessage, sizeof(InfiniteMessage), NULL, NULL);

        if (Console)
        {
            DWORD dwRead;
            INPUT_RECORD InputRecord;

            do
            { // Infinite loop waiting for valid input
                ReadConsoleInputW(hStdin, &InputRecord, 1, &dwRead);
            } while (IsInputInvalidate(InputRecord));
        } else NtWaitForMultipleObjects(1, &hStdin, WaitAll, FALSE, NULL);
    } else
    {
        char *p;
        char buffer[64];

        memcpy(buffer, _FiniteMessage, sizeof(_FiniteMessage));
        p = _ltoa10(timeout, buffer + sizeof(_FiniteMessage));
        memcpy(p, FiniteMessage_, sizeof(FiniteMessage_));

        if (Console)
        {
            DWORD dwRead;
            HANDLE Handles[2];
            INPUT_RECORD InputRecord;
            LARGE_INTEGER DelayInterval;

            NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock,
                buffer, p - buffer + sizeof(FiniteMessage_), NULL, NULL);
            NtCreateThreadEx(&Handles[0], THREAD_ALL_ACCESS, NULL,
                (HANDLE) -1, WaitForInput, &(THREAD_PARAMETER){hStdout, timeout},
                THREAD_CREATE_FLAGS_SKIP_THREAD_ATTACH | THREAD_CREATE_FLAGS_HIDE_FROM_DEBUGGER |
                THREAD_CREATE_FLAGS_SKIP_LOADER_INIT | THREAD_CREATE_FLAGS_BYPASS_PROCESS_FREEZE,
                0, 0, 0, NULL);
            Handles[1] = hStdin;
            NtQuerySystemTime(&DelayInterval);
            // Positive value for Absolute timeout
            DelayInterval.QuadPart += SecToUnits(timeout);

            do
            { // Wait for either timeout or input
                if (NtWaitForMultipleObjects(2, Handles, WaitAny,
                    FALSE, &DelayInterval) == STATUS_TIMEOUT) break;
                ReadConsoleInputW(hStdin, &InputRecord, 1, &dwRead);
            } while (IsInputInvalidate(InputRecord));

            NtClose(Handles[0]);
        } else
        {
            NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock,
                buffer, p - buffer + sizeof(FiniteMessage_) - 5, NULL, NULL);
            NtWaitForMultipleObjects(1, &hStdin, WaitAll, // Relative time
                FALSE, &(LARGE_INTEGER){.QuadPart=-SecToUnits(timeout)});
        }
    }

    // Simulate normal behavior
    NtWriteFile(hStdout, NULL, NULL, NULL,
        &IoStatusBlock, InfiniteMessage, 1, NULL, NULL);
}