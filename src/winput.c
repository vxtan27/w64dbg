/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

#include "resrc.h" // Resource
#include "ntdll.h" // Native

#define SecToUnits(lSeconds) ((lSeconds) * 10000000LL)

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
    HANDLE hStdin;
    long timeout;
} THREAD_PARAMETER;

static DWORD WINAPI WaitForInput(
  _In_ LPVOID lpParameter
)
{
    DWORD dwRead;
    INPUT_RECORD InputRecord;
    LARGE_INTEGER DelayInterval;
    THREAD_PARAMETER *Parameter = (THREAD_PARAMETER *) lpParameter;

    NtQuerySystemTime(&DelayInterval);
    // Positive value for Absolute timeout
    DelayInterval.QuadPart += SecToUnits(Parameter->timeout);

    do
    { // Wait for either timeout or input
        if (NtWaitForSingleObject(Parameter->hStdin, FALSE,
            &DelayInterval) == STATUS_TIMEOUT) break;
        ReadConsoleInputW(Parameter->hStdin, &InputRecord, 1, &dwRead);
    } while (IsInputInvalidate(InputRecord));

    ExitProcess(0);
}

static
__forceinline
char *__builtin_ltoa(
    _In_ long value,
    _Out_writes_(10) char *p
    )
{
    long num = value;

    // Pre-compute number Count
    while ((num /= 10)) ++p;

    char *ptr = p;

    do *ptr-- = (value % 10) + '0';
    while ((value /= 10));

    return p + 1;
}

static const char InfiniteMessage[30] = "\nPress any key to continue ...";
static const char FiniteMessage[42] = " seconds, press a key to continue ...\x1b[37D";

static
__forceinline
VOID WaitForInputOrTimeout(
    _In_ HANDLE hStdin,
    _In_ HANDLE hStdout,
    _In_ char StdinConsole,
    _In_ long timeout
    )
{
    IO_STATUS_BLOCK IoStatusBlock;

    if (timeout == -1)
    {
        // Write the InfiniteMessage
        NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock,
            InfiniteMessage, sizeof(InfiniteMessage), NULL, NULL);

        if (StdinConsole)
        {
            DWORD dwRead;
            INPUT_RECORD InputRecord;

            do
            { // Infinite loop waiting for valid input
                ReadConsoleInputW(hStdin, &InputRecord, 1, &dwRead);
            } while (IsInputInvalidate(InputRecord));
        } else NtWaitForSingleObject(hStdin, FALSE, // Relative time
            &(LARGE_INTEGER){.QuadPart=MAXLONGLONG});
    } else
    {
        char *p;
        char buffer[64] = "\nWaiting for ";

        p = __builtin_ltoa(timeout, buffer + 13);
        memcpy(p, FiniteMessage, sizeof(FiniteMessage));
        NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock,
            buffer, p - buffer + sizeof(FiniteMessage), NULL, NULL);

        if (StdinConsole)
        {
            long temp;
            ULONG Length;
            char Count, Recursive;

            CreateThread(NULL, 0, WaitForInput,
                &(THREAD_PARAMETER){hStdin, timeout}, 0, NULL);
            buffer[0] = '\b';

            while (TRUE)
            {
                NtDelayExecution(FALSE,
                    &(LARGE_INTEGER){.QuadPart=-(999 * 10000)});
                temp = --timeout;
                Count = 1;

                while (temp % 10 + '0' == '9')
                {
                    buffer[Count++] = '\b';
                    temp /= 10;
                }

                Recursive = Count;

                if (!temp)
                {
                    buffer[Count++] = ' ';
                    --Recursive;
                }

                temp = timeout;
                Length = Count + Recursive;

                while (Recursive)
                {
                    buffer[Count + --Recursive] = temp % 10 + '0';
                    temp /= 10;
                }

                NtWriteFile(hStdout, NULL, NULL, NULL,
                    &IoStatusBlock, buffer, Length, NULL, NULL);
            }
        } else NtWaitForSingleObject(hStdin, FALSE, // Relative time
            &(LARGE_INTEGER){.QuadPart=-SecToUnits(timeout)});
    }

    // Simulate normal behavior
    NtWriteFile(hStdout, NULL, NULL, NULL,
        &IoStatusBlock, InfiniteMessage, 1, NULL, NULL);
}