/* Copyright (c) 2024, vxtan27. Licensed under the BSD-3-Clause License. */

#include "ntdll.h" // Native

#define SecToUnits(lSeconds) (lSeconds * 10000000)

#define IsInputValidate(InputRecord) (InputRecord.EventType == KEY_EVENT && \
    InputRecord.Event.KeyEvent.bKeyDown && \
    InputRecord.Event.KeyEvent.wVirtualKeyCode != VK_MENU && \
    InputRecord.Event.KeyEvent.wVirtualKeyCode != VK_CONTROL)

static inline VOID WaitForInputOrTimeout(
    _In_ HANDLE hStdin,
    _In_ HANDLE hStdout,
    _In_ char StdinConsole,
    _In_ int timeout
    )
{
    IO_STATUS_BLOCK IoStatusBlock;

    NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock,
        "\nPress any key to continue ...", 30, NULL, NULL);

    if (StdinConsole)
    {
        DWORD dwRead;
        INPUT_RECORD InputRecord;

        if (timeout == -1) while (TRUE)
        {
            ReadConsoleInputA(hStdin, &InputRecord, 1, &dwRead);
            if (IsInputValidate(InputRecord)) break;
        } else
        {
            LARGE_INTEGER DelayInterval;

            NtQuerySystemTime(&DelayInterval);
            // Convert seconds to 100-nanosecond units
            // (positive for absolute time)
            DelayInterval.QuadPart += SecToUnits(timeout);

            while (TRUE)
            {
                if (NtWaitForSingleObject(hStdin, FALSE,
                    &DelayInterval) == STATUS_TIMEOUT) break;
                ReadConsoleInputA(hStdin, &InputRecord, 1, &dwRead);
                if (IsInputValidate(InputRecord)) break;
            }
        }
    } else
    {
        LARGE_INTEGER DelayInterval;

        // Convert seconds to 100-nanosecond units
        // (negative for relative time)
        DelayInterval.QuadPart = -SecToUnits(timeout);
        NtWaitForSingleObject(hStdin, FALSE, &DelayInterval);
    }

    // Simulate pause / timeout -1 behavior
    NtWriteFile(hStdout, NULL, NULL, NULL, &IoStatusBlock,
        "\nPress any key to continue ...", 1, NULL, NULL);
}